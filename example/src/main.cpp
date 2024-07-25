#include <result/result.hpp>

#include <string>

auto main() -> int
{
	std2::result<int, void> result = std2::ok(10);
	result = std2::err();
}
