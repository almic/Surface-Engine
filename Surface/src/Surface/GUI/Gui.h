#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <vector>

using namespace ImGui;

// Must forward declare this struct because fuck you
struct ImGuiDockContext
{
	ImGuiStorage Nodes;
};

namespace Surface {
namespace Gui {

	// These are TYPES and have nothing to do with actual position. These ONLY separate docking spaces. Remember, order matters!
	enum class WindowType : unsigned int
	{
		ANY = 0, // ImGui uses 0 as the "any" class, allowing other classes to dock together if one class has an ID of 0

		MAIN,
		ASIDE_LEFT,
		ASIDE_RIGHT,

		MAIN_LEFT,
		MAIN_RIGHT,
		MAIN_TOP,
		MAIN_BOTTOM,

		ASIDE_LEFT_2,
		ASIDE_LEFT_3,

		ASIDE_RIGHT_2,
		ASIDE_RIGHT_3

		// The reason there are this many groups is to keep things simple.
		// Remember that at the end of the day, the GUI should be simple and
		// easy to find what you need. Having 10 windows is insane when you
		// could easily have 3 or 4 sections with tabs.
	};

	static std::string WindowTypeToString(WindowType type)
	{
		switch (type)
		{
		case WindowType::ANY           : return "WindowType::ANY";
		case WindowType::MAIN          : return "WindowType::Main";
		case WindowType::ASIDE_LEFT    : return "WindowType::ASIDE_LEFT";
		case WindowType::ASIDE_RIGHT   : return "WindowType::ASIDE_RIGHT";
		case WindowType::MAIN_LEFT     : return "WindowType::MAIN_LEFT";
		case WindowType::MAIN_RIGHT    : return "WindowType::MAIN_RIGHT";
		case WindowType::MAIN_TOP      : return "WindowType::MAIN_TOP";
		case WindowType::MAIN_BOTTOM   : return "WindowType::MAIN_BOTTOM";
		case WindowType::ASIDE_LEFT_2  : return "WindowType::ASIDE_LEFT_2";
		case WindowType::ASIDE_LEFT_3  : return "WindowType::ASIDE_LEFT_3";
		case WindowType::ASIDE_RIGHT_2 : return "WindowType::ASIDE_RIGHT_2";
		case WindowType::ASIDE_RIGHT_3 : return "WindowType::ASIDE_RIGHT_3";
		default:
			SURF_CORE_ERROR("Unknown WindowType with value ({0}), please add string conversion to Gui::WindowTypeToString()!", (int)type);
			return "UNKNOWN TYPE";
		}
	}

	// More enums because having 3+ bools in a row is awful to look at.
	// This is really just to help change window behavior without using 5+ bool
	// parameters. These flags are NOT stored in the actual window as such, and
	// often relate to inverted bools with different names.
	// Like I said, this just helps change multiple settings in one go rather
	// than having to change all the bools individually.

	// Used to change multiple window settings at once, see the window class itself to change individual settings
	enum WindowFlags_ : int
	{
		WindowFlags_Unchanged          = -1,       // Window has default settings when initializing (see WindowFlags_Default), or uses original settings when building layouts
		WindowFlags_Hidden             = 0,        // Window starts hidden, only used when initializing
		WindowFlags_Visible            = 1 << 0,   // Window starts visible, only used when initializing
		WindowFlags_NoDockingUnclassed = 1 << 1,   // Prevents unclassed windows from being docked with this window
		WindowFlags_GroupWithSameType  = 1 << 2,   // When building layouts, this window will try to group with other windows of the same class rather than create a new split node
		WindowFlags_AutoHideTab        = 1 << 3,   // When building layouts, this window will automatically hide (minimize) its tab bar if its the only window
		WindowFlags_DisableClose       = 1 << 4,   // When building layouts, this window will forcefully remove the close button for its node, warnings will be logged if other windows are grouped and don't have this flag enabled
		WindowFlags_DisableCloseForce  = 1 << 5,   // Exactly like WindowFlags_DisableClose, except warnings will be supressed
		WindowFlags_DisableTab         = 1 << 6,   // When building layouts, this window will forcefully remove the tab bar completely for its node, warnings will be logged if other windows are grouped and don't have this flag enabled
		WindowFlags_DisableTabForce    = 1 << 7,   // Exactly like WindowFlags_DisableTab, except warnings will be supressed

		WindowFlags_Default            = WindowFlags_Hidden // Starts hidden, unclassed docking is enabled, dock tabs are shown
	};

	typedef int WindowFlags;

	struct WindowLayout
	{
		ImGuiDir Direction  = ImGuiDir_None;
		float    SplitRatio = 0.f;
	};

	// Because ImGui is cool, we can actually store some user data in our windows to retrieve later
	// Ok turns out you can't actually do that yet... to be continued.
	/*struct WindowData : ImGuiWindowClass
	{
		WindowLayout layout;
		const char*  name;
		bool         visible;
	};*/

	class Builder;

	class Window
	{
	public:
		WindowFlags  flags;
		WindowLayout layout;
		WindowType   type;
		const char*  name;
		bool         visible;
	
		//WindowData imGuiClass;
		ImGuiWindowClass imGuiClass;

		Window(const char*  name   = "Base Window",
		       WindowFlags  flags  = WindowFlags_Unchanged,
		       WindowType   type   = WindowType::ANY,
		       WindowLayout layout = { ImGuiDir_None, .5f })
			: name(name), flags(flags), layout(layout), type(type)
		{
			if (flags == WindowFlags_Unchanged)
				flags = WindowFlags_Default;

			visible = (bool)(flags & WindowFlags_Visible);

			imGuiClass.ClassId               =  (ImGuiID)type;
			imGuiClass.DockingAllowUnclassed = !(bool)(flags & WindowFlags_NoDockingUnclassed);
			//imGuiClass.layout                =  layout;
		}

		// ONLY CALLED IF WINDOW WOULD BE VISIBLE, return `true` if you made a window with ImGui::Begin() or `false` if not REGARDLESS OF `visible`. Should call ImGui::SetNextWindowClass(&imGuiClass) if you intend to dock the window
		inline virtual bool GuiBegin() {
			SetNextWindowClass(&imGuiClass);
			Begin(name, &visible);
			return true;
		}

		// Only do element rendering here, will not be called if the window is collapsed!
		inline virtual void Body(ImGuiWindow* window = nullptr) {}

		// Call ImGui::End() or similar here
		inline virtual void GuiEnd() { End(); }

		// Helper function to begin and end the GuiWindow, will always call GuiEnd() for you
		inline virtual void Show() final
		{
			if (visible)
			{
				bool is_window = GuiBegin();

				if (!is_window)
					Body();
				else
				{
					ImGuiWindow* window = FindWindowByName(name);

					if (window && (bool)(flags & (WindowFlags_DisableClose | WindowFlags_DisableCloseForce)))
						window->HasCloseButton = false;

					if (window && window->Collapsed)
						return GuiEnd();

					Body(window);
				}

				GuiEnd();
			}
		}
	private:
		static ImGuiID unique_class;

	protected:
		static ImGuiID GetUniqueClass()
		{
			return (unique_class--);
		}
	};

	class Builder
	{
	public:
		Builder() {}
		~Builder() {}

	private:
		// Holds the window groups to make dock nodes use consistent flags
		std::unordered_map<ImGuiID, std::pair<WindowFlags, std::vector<Window*>>> groups = {};

		// The active window that is having its layout built
		ImGuiID window_id   = 0;
		
		// Tracks the "current" location, which is always the most recently split or docked area
		ImGuiID active_id   = 0;

		ImGuiID dock_left   = 0;  // Tracks the most recently created LEFT   node
		ImGuiID dock_right  = 0;  // Tracks the most recently created RIGHT  node
		ImGuiID dock_top    = 0;  // Tracks the most recently created TOP    node
		ImGuiID dock_bottom = 0;  // Tracks the most recently created BOTTOM node

		// Used by Add and Split to determine if docking is happening
		bool building = false;

	public:

		// Please don't call this every frame with force=true, because it is very inefficient and you really shouldn't be rebuilding the entire layout every frame anyway!
		virtual bool        BeginLayout(ImGuiID windowID, bool force = false) final;

		// Must (should?) call this when you're finished docking
		virtual void        FinishLayout() final;

		// NOTE: Because this one part is getting out of control, I'm putting it on the back-burner.
		// It may be best to wait until the docking branch is merged into the main branch on dear imgui
		/*virtual bool        LoadLayout(const std::string &layout) final;  // Must be called AFTER adding windows, will overwrite layout styles
		virtual std::string SaveLayout() final;

		virtual std::vector<std::pair<std::string, std::string>> GetAllLayouts() final;
		virtual std::string LoadLayoutFile(const std::string& name) final;
		virtual bool        SaveLayoutFile(const std::string& layout, const std::string& name) final;*/

		// No two windows may share the same name, adds the window to the given node (or the ACTIVE node if unspecified) returns TRUE if the node existed and could be docked, or FALSE if it was not yet created or has been split.
		virtual bool  Add         (Window* window, ImGuiID nodeID = 0, WindowFlags flags = WindowFlags_Unchanged, WindowType type = WindowType::ANY, bool forceType = false) final;
		virtual bool  AddLeft     (Window* window,                     WindowFlags flags = WindowFlags_Unchanged, WindowType type = WindowType::ANY, bool forceType = false) final { return Add(window, dock_left,   flags, type, forceType); } // Adds window to the last set LEFT   node, and setting this node as the ACTIVE node, failing if the node is not yet created, or was destroyed by a prior vertical   split
		virtual bool  AddRight    (Window* window,                     WindowFlags flags = WindowFlags_Unchanged, WindowType type = WindowType::ANY, bool forceType = false) final { return Add(window, dock_right,  flags, type, forceType); } // Adds window to the last set RIGHT  node, and setting this node as the ACTIVE node, failing if the node is not yet created, or was destroyed by a prior vertical   split
		virtual bool  AddTop      (Window* window,                     WindowFlags flags = WindowFlags_Unchanged, WindowType type = WindowType::ANY, bool forceType = false) final { return Add(window, dock_top,    flags, type, forceType); } // Adds window to the last set TOP    node, and setting this node as the ACTIVE node, failing if the node is not yet created, or was destroyed by a prior horizontal split
		virtual bool  AddBottom   (Window* window,                     WindowFlags flags = WindowFlags_Unchanged, WindowType type = WindowType::ANY, bool forceType = false) final { return Add(window, dock_bottom, flags, type, forceType); } // Adds window to the last set BOTTOM node, and setting this node as the ACTIVE node, failing if the node is not yet created, or was destroyed by a prior horizontal split

		// Splits the given node (or the ACTIVE node if unspecified) with the direction having size RATIO. Returns a pair of the new nodes, the given direction first and opposite second, and sets the ACTIVE node to the given direction.
		virtual std::pair<ImGuiID, ImGuiID>  Split       (ImGuiDir direction, float ratio = .5f, ImGuiID nodeID = 0) final;
		virtual std::pair<ImGuiID, ImGuiID>  SplitLeft   (                    float ratio = .5f, ImGuiID nodeID = 0) final { return Split(ImGuiDir_Left,  ratio, nodeID); } // Fails if the node already has docked windows, returns [left, right], splits the given or ACTIVE node horizontally, with the LEFT   node having size RATIO. Internally sets the ACTIVE node to the LEFT   node, as well as the LEFT and RIGHT nodes respectively. Unsets the TOP or BOTTOM internal node if it was split.
		virtual std::pair<ImGuiID, ImGuiID>  SplitRight  (                    float ratio = .5f, ImGuiID nodeID = 0) final { return Split(ImGuiDir_Right, ratio, nodeID); } // Fails if the node already has docked windows, returns [right, left], splits the given or ACTIVE node horizontally, with the RIGHT  node having size RATIO. Internally sets the ACTIVE node to the RIGHT  node, as well as the LEFT and RIGHT nodes respectively. Unsets the TOP or BOTTOM internal node if it was split.
		virtual std::pair<ImGuiID, ImGuiID>  SplitTop    (                    float ratio = .5f, ImGuiID nodeID = 0) final { return Split(ImGuiDir_Up,    ratio, nodeID); } // Fails if the node already has docked windows, returns [top, bottom], splits the given or ACTIVE node vertically,   with the TOP    node having size RATIO. Internally sets the ACTIVE node to the TOP    node, as well as the TOP and BOTTOM nodes respectively. Unsets the LEFT or RIGHT internal node if it was split.
		virtual std::pair<ImGuiID, ImGuiID>  SplitBottom (                    float ratio = .5f, ImGuiID nodeID = 0) final { return Split(ImGuiDir_Down,  ratio, nodeID); } // Fails if the node already has docked windows, returns [bottom, top], splits the given or ACTIVE node vertically,   with the BOTTOM node having size RATIO. internally sets the ACTIVE node to the BOTTOM node, as well as the TOP and BOTTOM nodes respectively. Unsets the LEFT or RIGHT internal node if it was split.
	
	//private:
		// This helps SaveLayout() by recursively collecting docked window data
		//virtual void CollectNodes(std::vector<WindowData>* dc, ImGuiDockNode* node, int depth) final;
	};

}
}
