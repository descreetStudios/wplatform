#pragma once

#include <wcore/types.hpp>

namespace wplatform
{
	struct Extent
	{
		u32 width;
		u32 height;
	};

	struct Position
	{
		i32 x;
		i32 y;
	};

	struct Scale
	{
		f32 x;
		f32 y;
	};
} // namespace wplatform
