DXWindow
=========
DXWindow is a small layer over DXGI written for C++ applications that
simplifies the process of creating and using a window.  Think of it as
GLUT for DirectX.  As a window-specific library, it makes use of the
COM ABI.  Its intent is to provide a fast, easy way to create a window
that behaves well.

What it does.
-------------
DXWindow absolves an application developer of the responsibility of learning
the more frustrating (yet necessary) intricacies of DXGI.  Rather than wasting
time writing windowing code that behaves the same as just about every other
game out there, it lets you skip right to the graphics.

Some interesting points:

- DXWindow is device-agnostic, so long as the device cooperates with DXGI - this means you can use any version of Direct3D between 10 and 12 (maybe even 13 and beyond).  Just provide the device as an `IUnknown` interface, and DXWindow does the rest.
- DXWindow handles movement between monitors so that proper functionality isn't just limited to the primary output device.
- DXWindow supports all of the common window modes used in games: exclusive fullscreen, fullscreen window, windowed, and borderless.
- DXWindow is interoperable with Direct2D as well, as it uses the BGRA texture format that Direct2D render targets require.
- DXWindow takes care of the most common forms of game input - keyboard, mouse, and gamepad.

The library abstracts all window behavior away from the application developer.  This way, you only ever have to worry about what's happening in your game and not the window.

What it doesn't do.
-------------
DXWindow does not attempt to provide complete control over window setup.  It handles only the most common use cases, and does not make itself easily extensible without modification of the source code.  It also does not provide a way to make the back buffer MSAA-enabled - the reason for this is that it makes for a simpler API, and you can still make a secondary buffer that supports MSAA and resolve it to the back buffer anyway.  This is what DXGI does in the background, so there are no added inefficiencies from doing things this way.  DXWindow also does not handle multiple adapters.  It uses whatever adapter the device that you hand it occupies, and does not alert you when the window has moved between adapters.  However, this occurrence is very, very uncommon, as most gaming and non-gaming setups provide only one virtualized adapter when SLI or Crossfire are considered.

Usage
-------------
A window can be created in a few simple steps:

#### 1. Create the window description

`DXWINDOW_DESC` is a structure describing the initial setup of the window:

    struct DXWINDOW_DESC {
        LPCWSTR IconSmall;
        LPCWSTR IconLarge;
        LPCWSTR Cursor;
        LPCWSTR Title;
        HINSTANCE Instance;
        DXWINDOW_FULLSCREEN_STATE FullscreenState;
        DXWINDOW_WINDOW_STATE WindowState;
        WORD Width;
        WORD Height;
        BOOL InitFullscreen;
        BOOL AllowToggle;
    };

- `IconSmall`, `IconLarge`, and `Cursor` are all resources.  The former two asign the icon of the application's window, while the latter assigns the cursor that appears when the mouse is in window focus.  These cannot be changed once the window has been created.  If set to NULL, DXWindow will choose the default setup for each of these items.
- `Title` is the name of the window.  This also cannot be changed.
- `Instance` is the process `HINSTANCE` handle.
- `FullscreenState` is the type of fullscreen the window should use when it's in fullscreen mode.  This can be one of either `DXWINDOW_FULLSCREEN_STATE_FULLSCREEN` or `DXWINDOW_FULLSCREEN_STATE_FULLSCREEN_WINDOW`.
- Conversely, `WindowState` is the type of window layout that should be used when the window is not in fullscreen mode.  It can be one of either `DXWINDOW_WINDOW_STATE_WINDOWED` or `DXWINDOW_WINDOW_STATE_BORDERLESS`.
- `Width` and `Height` are the dimensions of the client area of the window when it's in a windowed mode.  DXWindow will automatically adjust the full dimensions of the window when the border is added or removed when moving between window modes.
- `InitFullscreen` is a flag indicating whether or not the window should enter its fullscreen state upon creation.
- Finally, `AllowToggle` is a flag indicating whether or not the window should toggle between its window and fullscreen states when the user presses F11.
