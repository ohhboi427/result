#include <result/result.hpp>

auto main() -> int
{
	std2::result<int, void> result = std2::err();

	auto i = result
		.transform([] (int& value) -> float
				   {
					   return value * 2.0f;
				   })
		.ok();
}
