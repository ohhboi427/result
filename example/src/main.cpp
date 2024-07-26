#include <result/result.hpp>

auto main() -> int
{
	std2::result<int, void> result = std2::err();

	auto i = result
		.and_then([] (int& value) -> std2::result<float, void>
				  {
					  return std2::ok(10.0f);
				  })
		.ok_or(0);
}
