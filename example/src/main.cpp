#include <result/result.hpp>

#include <optional>

auto main() -> int
{
	std2::result<int, void> result = std2::err();
	auto i = result.ok_or(10);
}
