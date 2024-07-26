#pragma once

#include <concepts>
#include <format>
#include <functional>
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
	class result;

	template<typename F, typename E, typename... Args>
	struct is_invoke_result_result_with_err : std::bool_constant<std::conjunction_v<std::_Is_specialization<std::invoke_result_t<F, Args...>, result>, std::is_same<typename std::invoke_result_t<F, Args...>::err_type, E>>> {};

	template<typename F, typename E, typename... Args>
	inline constexpr bool is_invoke_result_result_with_err_v = is_invoke_result_result_with_err<F, E, Args...>::template value;

	template<typename F, typename E, typename... Args>
	concept invoke_result_result_with_err = is_invoke_result_result_with_err_v<F, E, Args...>;

	template<typename F, typename T, typename... Args>
	struct is_invoke_result_result_with_ok : std::bool_constant<std::conjunction_v<std::_Is_specialization<std::invoke_result_t<F, Args...>, result>, std::is_same<typename std::invoke_result_t<F, Args...>::ok_type, T>>> {};

	template<typename F, typename T, typename... Args>
	inline constexpr bool is_invoke_result_result_with_ok_v = is_invoke_result_result_with_ok<F, T, Args...>::template value;

	template<typename F, typename T, typename... Args>
	concept invoke_result_result_with_ok = is_invoke_result_result_with_ok_v<F, T, Args...>;

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

		template<std::invocable<> F>
			requires std::conjunction_v<std::is_void<T>, is_invoke_result_result_with_err<F, E>>
		[[nodiscard]] constexpr auto and_then(F&& func) &
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F>, std::is_nothrow_invocable<decltype(std2::err<std::add_lvalue_reference_t<E>>), result_storage<E>&>>)
			-> std::invoke_result_t<F>
		{
			if(m_is_ok)
			{
				return std::invoke(func);
			}

			return std2::err<std::add_lvalue_reference_t<E>>(m_err);
		}

		template<std::invocable<result_storage<T>&> F>
			requires std::conjunction_v<std::negation<std::is_void<T>>, is_invoke_result_result_with_err<F, E, result_storage<T>&>>
		[[nodiscard]] constexpr auto and_then(F&& func) &
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F, result_storage<T>&>, std::is_nothrow_invocable<decltype(std2::err<std::add_lvalue_reference_t<E>>), result_storage<E>&>>)
			-> std::invoke_result_t<F, result_storage<T>&>
		{
			if(m_is_ok)
			{
				return std::invoke(func, m_ok);
			}

			return std2::err<std::add_lvalue_reference_t<E>>(m_err);
		}

		template<std::invocable<> F>
			requires std::conjunction_v<std::is_void<T>, is_invoke_result_result_with_err<F, E>>
		[[nodiscard]] constexpr auto and_then(F&& func) const&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F>, std::is_nothrow_invocable<decltype(std2::err<std::add_lvalue_reference_t<const E>>), const result_storage<E>&>>)
			-> std::invoke_result_t<F>
		{
			if(m_is_ok)
			{
				return std::invoke(func);
			}

			return std2::err<std::add_lvalue_reference_t<const E>>(m_err);
		}

		template<std::invocable<const result_storage<T>&> F>
			requires std::conjunction_v<std::negation<std::is_void<T>>, is_invoke_result_result_with_err<F, E, const result_storage<T>&>>
		[[nodiscard]] constexpr auto and_then(F&& func) const&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F, const result_storage<T>&>, std::is_nothrow_invocable<decltype(std2::err<std::add_lvalue_reference_t<const E>>), const result_storage<E>&>>)
			-> std::invoke_result_t<F, const result_storage<T>&>
		{
			if(m_is_ok)
			{
				return std::invoke(func, m_ok);
			}

			return std2::err<std::add_lvalue_reference_t<const E>>(m_err);
		}

		template<std::invocable<> F>
			requires std::conjunction_v<std::is_void<T>, is_invoke_result_result_with_err<F, E>>
		[[nodiscard]] constexpr auto and_then(F&& func) &&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F>, std::is_nothrow_invocable<decltype(std2::err<E>), result_storage<E>&&>>)
			-> std::invoke_result_t<F>
		{
			if(m_is_ok)
			{
				return std::invoke(func);
			}

			return std2::err<E>(std::move(m_err));
		}

		template<std::invocable<result_storage<T>&&> F>
			requires std::conjunction_v<std::negation<std::is_void<T>>, is_invoke_result_result_with_err<F, E, result_storage<T>&&>>
		[[nodiscard]] constexpr auto and_then(F&& func) &&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F, result_storage<T>&&>, std::is_nothrow_invocable<decltype(std2::err<E>), result_storage<E>&&>>)
			-> std::invoke_result_t<F, result_storage<T>&&>
		{
			if(m_is_ok)
			{
				return std::invoke(func, std::move(m_ok));
			}

			return std2::err<E>(std::move(m_err));
		}

		template<std::invocable<> F>
			requires std::conjunction_v<std::is_void<T>, is_invoke_result_result_with_err<F, E>>
		[[nodiscard]] constexpr auto and_then(F&& func) const&&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F>, std::is_nothrow_invocable<decltype(std2::err<const E>), const result_storage<E>&&>>)
			-> std::invoke_result_t<F>
		{
			if(m_is_ok)
			{
				return std::invoke(func);
			}

			return std2::err<E>(std::move(m_err));
		}

		template<std::invocable<const result_storage<T>&&> F>
			requires std::conjunction_v<std::negation<std::is_void<T>>, is_invoke_result_result_with_err<F, E, const result_storage<T>&&>>
		[[nodiscard]] constexpr auto and_then(F&& func) const&&
			noexcept(std::conjunction_v<std::is_nothrow_invocable<F, const result_storage<T>&&>, std::is_nothrow_invocable<decltype(std2::err<const E>), const result_storage<E>&&>>)
			-> std::invoke_result_t<F, const result_storage<T>&&>
		{
			if(m_is_ok)
			{
				return std::invoke(func, std::move(m_ok));
			}

			return std2::err<E>(std::move(m_err));
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

	template<typename T, typename E, typename CharT>
		requires std::conjunction_v<std::is_default_constructible<formatter<T, CharT>>, std::is_default_constructible<formatter<E, CharT>>>
	struct formatter<std2::result<T, E>, CharT>
	{
		template<typename FormatContext>
		auto format(const std2::result<T, E>& result, FormatContext& context) const noexcept -> typename FormatContext::iterator
		{
			if(result.is_ok())
			{
				return std::format_to(context.out(), "ok{{{}}}", result.ok());
			}

			return std::format_to(context.out(), "err{{{}}}", result.err());
		}

		template<typename ParseContext>
		constexpr auto parse(ParseContext& context) noexcept -> typename ParseContext::iterator
		{
			return context.end();
		}
	};
}
