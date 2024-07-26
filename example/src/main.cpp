#include <result/result.hpp>

auto main() -> int
{
	std2::result<int, void> result = std2::err();

	auto i = result
		.or_else([] () -> std2::result<int, char>
				  {
					  return std2::ok(10);
				  })
		.ok_or(0);
}
