#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace std2
{
	template<typename T, bool IsOk>
	struct result_value
	{
		T value;
	};

	template<typename T>
	using ok_value = result_value<T, true>;

	template<typename E>
	using err_value = result_value<E, false>;

	template<typename T>
	[[nodiscard]] constexpr auto ok(T&& value) noexcept -> ok_value<std::decay_t<std::remove_reference_t<T>>>
	{
		return ok_value{ std::forward<T>(value) };
	}

	template<typename E>
	[[nodiscard]] constexpr auto err(E&& value) noexcept -> err_value<std::decay_t<std::remove_reference_t<E>>>
	{
		return err_value{ std::forward<E>(value) };
	}

	template<typename T, typename E>
	class result
	{
	public:
		using ok_type = T;
		using ere_type = E;

		template<std::convertible_to<T> U>
		constexpr result(ok_value<U>&& ok) noexcept(std::is_nothrow_constructible_v<T, std::add_rvalue_reference_t<U>>)
			: m_ok(std::move(ok.value)), m_is_ok{ true }
		{}

		template<std::convertible_to<E> F>
		constexpr result(err_value<F>&& err) noexcept(std::is_nothrow_constructible_v<E, std::add_rvalue_reference_t<F>>)
			: m_err(std::move(err.value)), m_is_ok{ false }
		{}

		constexpr ~result() noexcept(std::conjunction_v<std::is_nothrow_destructible<T>, std::is_nothrow_destructible<E>>)
		{
			if(m_is_ok)
			{
				m_ok.~T();
			}
			else
			{
				m_err.~E();
			}
		}

	private:
		union
		{
			T m_ok;
			E m_err;
		};
		bool m_is_ok;
	};
}
