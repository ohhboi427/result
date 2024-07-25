#include <result/result.hpp>

#include <format>
#include <iostream>

auto main() -> int
{
	std2::result<int, int> result = std2::ok(10);

	std::cout << std::format("{}\n", result);
}
