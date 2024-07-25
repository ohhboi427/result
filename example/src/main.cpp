#include <result/result.hpp>

#include <string>

auto main() -> int
{
	std2::result<int, std::string> result = std2::err("test");
}
