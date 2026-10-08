#pragma once

#include "core/window/window_backend.h"
#include "core/window/window_types.h"

namespace eui::window {

using ContextKey = core::window::ContextKey;
using CursorHandle = core::window::CursorHandle;
using CursorType = core::window::CursorType;
using Handle = core::window::Handle;

// Reports whether the created framebuffer actually carries alpha (GLFW honors
// the transparent request; SDL2 and compositor-less X11 do not).
using core::window::framebufferTransparent;

} // namespace eui::window
