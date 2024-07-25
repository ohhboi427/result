#include <result/result.hpp>

auto main() -> int
{
	std2::result<int, int> result = std2::err(20);
	auto i = result.ok_or(10);

	auto h = std::hash<decltype(result)>{}(result);
}
