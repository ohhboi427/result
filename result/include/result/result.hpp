#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace std2
{
	template<typename T>
	using result_storage = std::conditional_t<std::is_void_v<T>, uint8_t, T>;

	template<typename T, bool IsOk>
	struct result_value
	{
		result_storage<T> value;
	};

	template<typename T>
	using ok_value = result_value<T, true>;

	template<typename E>
	using err_value = result_value<E, false>;

	template<typename T = void>
		requires std::is_void_v<T>
	[[nodiscard]] constexpr auto ok(...) noexcept -> ok_value<void>
	{
		return ok_value<void>{};
	}

	template<typename T>
		requires std::negation_v<std::is_void<T>>
	[[nodiscard]] constexpr auto ok(T&& value)
		noexcept(std::is_nothrow_constructible_v<ok_value<std::decay_t<std::remove_reference_t<T>>>, T&&>)
		-> ok_value<std::decay_t<std::remove_reference_t<T>>>
	{
		return ok_value<std::decay_t<std::remove_reference_t<T>>>{ std::forward<T>(value) };
	}

	template<typename E = void>
		requires std::is_void_v<E>
	[[nodiscard]] constexpr auto err(...) noexcept -> err_value<void>
	{
		return err_value<void>{};
	}

	template<typename E>
		requires std::negation_v<std::is_void<E>>
	[[nodiscard]] constexpr auto err(E&& value)
		noexcept(std::is_nothrow_constructible_v<err_value<std::decay_t<std::remove_reference_t<E>>>, E&&>)
		-> err_value<std::decay_t<std::remove_reference_t<E>>>
	{
		return err_value<std::decay_t<std::remove_reference_t<E>>>{ std::forward<E>(value) };
	}

	template<typename T, typename E>
	class result
	{
	public:
		using ok_type = T;
		using err_type = E;

		template<std::convertible_to<T> U>
		constexpr result(ok_value<U>&& ok)
			noexcept(std::is_nothrow_convertible_v<std::add_rvalue_reference_t<U>, T>)
			: m_ok(std::move(ok.value)), m_is_ok{ true }
		{}

		template<std::convertible_to<E> F>
		constexpr result(err_value<F>&& err)
			noexcept(std::is_nothrow_convertible_v<std::add_rvalue_reference_t<F>, E>)
			: m_err(std::move(err.value)), m_is_ok{ false }
		{}

		result(const result&) = delete;
		result(result&&) noexcept = delete;

		constexpr ~result()
			noexcept(std::conjunction_v<std::is_nothrow_destructible<T>, std::is_nothrow_destructible<E>>)
		{
			if(m_is_ok)
			{
				m_ok.~decltype(m_ok)();
			}
			else
			{
				m_err.~decltype(m_err)();
			}
		}

		auto operator=(const result&) -> result& = delete;
		auto operator=(result&&) noexcept -> result& = delete;

		[[nodiscard]] constexpr operator bool() const noexcept
		{
			return m_is_ok;
		}

		[[nodiscard]] constexpr auto is_ok() const noexcept -> bool
		{
			return m_is_ok;
		}

		[[nodiscard]] constexpr auto is_err() const noexcept -> bool
		{
			return !m_is_ok;
		}

		template<typename = void>
			requires std::negation_v<std::is_void<T>>
		[[nodiscard]] constexpr auto ok() & noexcept -> result_storage<T>&
		{
			return m_ok;
		}

		template<typename = void>
			requires std::negation_v<std::is_void<T>>
		[[nodiscard]] constexpr auto ok() const& noexcept -> const result_storage<T>&
		{
			return m_ok;
		}

		template<typename = void>
			requires std::negation_v<std::is_void<T>>
		[[nodiscard]] constexpr auto ok() && noexcept -> result_storage<T>&&
		{
			return std::move(m_ok);
		}

		template<typename = void>
			requires std::negation_v<std::is_void<T>>
		[[nodiscard]] constexpr auto ok() const&& noexcept -> const result_storage<T>&&
		{
			return std::move(m_ok);
		}

		template<std::convertible_to<T> U>
			requires std::conjunction_v<std::negation<std::is_void<T>>, std::is_copy_constructible<T>>
		[[nodiscard]] constexpr auto ok_or(U&& def) const&
			noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<T>, std::is_nothrow_convertible<U&&, T>>)
			-> T
		{
			return m_is_ok
				? m_ok
				: std::forward<U>(def);
		}

		template<std::convertible_to<T> U>
			requires std::conjunction_v<std::negation<std::is_void<T>>, std::is_move_constructible<T>>
		[[nodiscard]] constexpr auto ok_or(U&& def) &&
			noexcept(std::conjunction_v<std::is_nothrow_move_constructible<T>, std::is_nothrow_convertible<U&&, T>>)
			-> T
		{
			return m_is_ok
				? std::move(m_ok)
				: std::forward<U>(def);
		}

		template<typename = void>
			requires std::negation_v<std::is_void<E>>
		[[nodiscard]] constexpr auto err() & noexcept -> result_storage<E>&
		{
			return m_err;
		}

		template<typename = void>
			requires std::negation_v<std::is_void<E>>
		[[nodiscard]] constexpr auto err() const& noexcept -> const result_storage<E>&
		{
			return m_err;
		}

		template<typename = void>
			requires std::negation_v<std::is_void<E>>
		[[nodiscard]] constexpr auto err() && noexcept -> result_storage<E>&&
		{
			return std::move(m_err);
		}

		template<typename = void>
			requires std::negation_v<std::is_void<E>>
		[[nodiscard]] constexpr auto err() const&& noexcept -> const result_storage<E>&&
		{
			return std::move(m_err);
		}

		template<std::convertible_to<E> F>
			requires std::conjunction_v<std::negation<std::is_void<E>>, std::is_copy_constructible<E>>
		[[nodiscard]] constexpr auto err_or(F&& def) const&
			noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<E>, std::is_nothrow_convertible<F&&, E>>)
			-> E
		{
			return !m_is_ok
				? m_err
				: std::forward<F>(def);
		}

		template<std::convertible_to<E> F>
			requires std::conjunction_v<std::negation<std::is_void<E>>, std::is_move_constructible<E>>
		[[nodiscard]] constexpr auto err_or(F&& def) &&
			noexcept(std::conjunction_v<std::is_nothrow_move_constructible<E>, std::is_nothrow_convertible<F&&, E>>)
			-> E
		{
			return !m_is_ok
				? std::move(m_err)
				: std::forward<F>(def);
		}

	private:
		union
		{
			result_storage<T> m_ok;
			result_storage<E> m_err;
		};
		bool m_is_ok;
	};
}

namespace std
{
	template<typename T, typename E>
		requires std::conjunction_v<std::is_default_constructible<hash<T>>, std::is_default_constructible<hash<E>>>
	struct hash<std2::result<T, E>>
	{
		[[nodiscard]] constexpr auto operator()(const std2::result<T, E>& result) const noexcept -> size_t
		{
			if(result.is_ok())
			{
				return hash<T>{}(result.ok());
			}

			return hash<E>{}(result.err());
		}
	};
}
