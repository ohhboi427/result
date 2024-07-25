#include <result/result.hpp>

#include <optional>

auto main() -> int
{
	std2::result<int, void> result = std2::ok(10);
	result.ok();
}
