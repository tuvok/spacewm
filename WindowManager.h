#include <memory>

#include <X11/Xlib.h>

class WindowManager
{
public:

    /**
     * Create instance of WindowManager
     */
    static std::unique_ptr<WindowManager> create();

    /**
     * Destroy instance
     */
    ~WindowManager();

    /**
     * Start the event loop
     */
    void run();

private:
    /**
     * Construct WindowManager object
     * @param _display XLib Display structure
     */
    WindowManager(std::shared_ptr<Display> _display);

    void reparentExistingWindows();
    void reparentWindow(Window* w, bool preExisting);

    void onCreateNotify(XCreateWindowEvent& e);
    void onMapRequest(XMapRequestEvent& e);
    void onConfigureRequest(XConfigureRequestEvent& e);

    static int handleError(Display* display, XErrorEvent* e);

private:
    static bool xRedirectFailed;
    std::shared_ptr<Display> display;
    const Window rootWindow;

};
