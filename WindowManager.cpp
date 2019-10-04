#include <WindowManager.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xutil.h>

#include <exception>

#include <iostream>

bool WindowManager::xRedirectFailed = false;

std::unique_ptr<WindowManager> WindowManager::create()
{
    /// Open connection to display
    ///
    Display* d = XOpenDisplay(nullptr);
    std::shared_ptr<Display> display(d,
                                     [](auto p)
                                     {
                                         XCloseDisplay(p);
                                     });

    if (!display)
    {
        throw std::runtime_error("Failed to open display");
    }

    /// Check support for Compositor extension
    ///
    int version = XCompositeVersion();
    int major = version / 1000;
    int minor = (version - (major * 1000)) / 100;
    int revision = version - (major * 1000) - (minor * 100);

    std::cout << "Composite Version: " << major << "." << minor << "." << revision << std::endl;

    auto status = XCompositeQueryVersion(d, &major, &minor);

    std::cout << "Composite Server Support: " << major << "." << minor << std::endl;

    if (!status)
    {
        throw std::runtime_error("Compositor version query failed");
    }

    return std::unique_ptr<WindowManager>(new WindowManager(std::move(display)));
}

WindowManager::WindowManager(std::shared_ptr<Display> _display) :
    display(std::move(_display)),
    rootWindow(DefaultRootWindow(display.get()))
{
    /// Grab root window events
    XSetErrorHandler(&WindowManager::handleError);
    int r = XSelectInput(display.get(), rootWindow, SubstructureRedirectMask | SubstructureNotifyMask);

    XSync(display.get(), false);
    if (!r || xRedirectFailed)
    {
        std::cout << "Failed to redirect root window events, "
            "probably some WM is already running" << std::endl;
        throw std::runtime_error("Failed to redirect root window events");
    }

    reparentExistingWindows();
}

void WindowManager::reparentExistingWindows()
{
    int r = XGrabServer(display.get());
    if (!r)
        std::cout << "XGrabServer -> " << r << std::endl;


    Window root, parent;
    Window* children;
    unsigned int count;
    r = XQueryTree(display.get(), rootWindow, &root, &parent, &children, &count);
    if (!r)
        std::cout << "XQueryTree -> " << r << std::endl;

    // FIXME: probably should be done on all screens?
    for (unsigned int i = 0; i < count; ++i)
    {
        //reparentWindow(&(children[i]), true);
    }

    /// Composite Redirect all screens' root windows
    for (int i = 0; i < ScreenCount(display.get()); ++i)
    {
        XCompositeRedirectSubwindows(display.get(),
                                     RootWindow(display.get(), i),
                                     CompositeRedirectAutomatic);
    }


    XFree(children);

    r = XUngrabServer(display.get());
    if (!r)
        std::cout << "XUngrabServer -> " << r << std::endl;
}

void WindowManager::reparentWindow(Window* w, bool preExisting)
{
    // We shouldn't be framing windows we've already framed.
    //CHECK(!clients_.count(w));

    XWindowAttributes windowAttrib;
    XGetWindowAttributes(display.get(), *w, &windowAttrib);

    /// If window existed before WM was started:
    if (preExisting)
    {
        if (windowAttrib.override_redirect || windowAttrib.map_state != IsViewable)
        {
            std::cout << "Skipping window" << std::endl;
            return;
        }
    }


/*
    const Window frame = XCreateSimpleWindow(
      display_,
      root_,
      x_window_attrs.x,
      x_window_attrs.y,
      x_window_attrs.width,
      x_window_attrs.height,
      BORDER_WIDTH,
      BORDER_COLOR,
      BG_COLOR);
  // 4. Select events on frame.
  XSelectInput(
      display_,
      frame,
      SubstructureRedirectMask | SubstructureNotifyMask);
  // 5. Add client to save set, so that it will be restored and kept alive if we
  // crash.
  XAddToSaveSet(display_, w);
  // 6. Reparent client window.
  XReparentWindow(
      display_,
      w,
      frame,
      0, 0);  // Offset of client window within frame.
  // 7. Map frame.
  XMapWindow(display_, frame);
  // 8. Save frame handle.
  clients_[w] = frame;
  // 9. Grab universal window management actions on client window.
  //   a. Move windows with alt + left button.
  XGrabButton(
      display_,
      Button1,
      Mod1Mask,
      w,
      false,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   b. Resize windows with alt + right button.
  XGrabButton(
      display_,
      Button3,
      Mod1Mask,
      w,
      false,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   c. Kill windows with alt + f4.
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_F4),
      Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  //   d. Switch windows with alt + tab.
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
*/

}

int WindowManager::handleError(Display* display, XErrorEvent* e)
{
    switch (e->error_code)
    {
        case BadAccess:
        {
            std::cout << "X11 error: BadAccess: " << (int)e->request_code << std::endl;
            xRedirectFailed = true;
        }; break;

        default:
        {
            std::cout << "Unhandled error: " << e->error_code << std::endl;
        }
    }

    return 0;
}

WindowManager::~WindowManager()
{

}

void WindowManager::run()
{

    while(true)
    {
        XEvent e;
        XNextEvent(display.get(), &e);

        std::cout << "Received event: " << e.type << std::endl;


        switch (e.type)
        {
            case CreateNotify:
                onCreateNotify(e.xcreatewindow);
                break;
            case MapRequest:
                onMapRequest(e.xmaprequest);
                break;
            case ConfigureRequest:
                onConfigureRequest(e.xconfigurerequest);
                break;
            default:
                std::cout << "Unhandled event: " << e.type << std::endl;
        }
    }

}

void WindowManager::onCreateNotify(XCreateWindowEvent& e)
{

}

void WindowManager::onMapRequest(XMapRequestEvent& e)
{
    // add frame

    XMapWindow(display.get(), e.window);
}

void WindowManager::onConfigureRequest(XConfigureRequestEvent& e)
{
    XWindowChanges c;
    c.x = e.x;
    c.y = e.y;
    c.width = e.width;
    c.height = e.height;
    c.border_width = e.border_width;
    c.sibling = e.above;
    c.stack_mode = e.detail;

    //if (clients_.count(e.window))
//    {
//        const Window frame = clients_[e.window];
//        XConfigureWindow(display_, frame, e.value_mask, &changes);
//        LOG(INFO) << "Resize [" << frame << "] to " << Size<int>(e.width, e.height);
//    }

    XConfigureWindow(display.get(), e.window, e.value_mask, &c);
}
