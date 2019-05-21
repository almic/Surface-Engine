#include "spch.h"

#include "Gui.h"

namespace Surface {
	namespace Gui {

		ImGuiID Window::unique_class = -1;

		bool Builder::BeginLayout(ImGuiID windowID, bool force)
		{
			// Check that current window_id is 0
			if (window_id != 0)
			{
				SURF_CORE_WARN("Unexpected call to Builder::BeginLayout() before calling Builder::FinishLayout()! Please make sure you have finished the previous layout before starting a new one.");
				return false;
			}

			// Only set layout if not already set or if `force` is true
			if (!force && DockBuilderGetNode(windowID) != NULL)
				return false;
			else
				DockBuilderRemoveNode(windowID);

			DockBuilderAddNode(windowID, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_CentralNode);

			active_id = window_id = windowID;

			// Used by all other functions to know if docking is happening
			building = true;
			return true;
		}

		void Builder::FinishLayout()
		{
			building = false;
			DockBuilderFinish(window_id);
			window_id = active_id = 0;
			dock_left = dock_right = dock_top = dock_bottom = 0;
			groups.clear();
		}

		bool Builder::Add(Window* window, ImGuiID nodeID, WindowFlags flags, WindowType type, bool forceType)
		{
			// Check that we are actually building
			if (!building)
			{
				SURF_CORE_ERROR("Gui::Builder::Add() failed! Make sure you only call docking functions if BeginLayout() returns true!");
				return false;
			}

			// Check window isn't already docked
			for (auto[dockID, group] : groups)
			{
				for (Window* other : group.second)
				{
					if (other->name == window->name)
					{
						//SURF_CORE_WARN("Gui::Builder::Add() failed, window with name \"{0}\" already docked!", window->name);
						// Maybe we want to double-check that window was already added?
						return false;
					}
				}
			}

			if (nodeID == 0) nodeID = active_id;

			// Make sure node is not split
			ImGuiDockNode* node = DockBuilderGetNode(nodeID);

			if (node->IsSplitNode())
			{
				// There is not a different message in case the "active_id" was chosen, because by definition it is impossible for the "active_id" to be a split node.
				SURF_CORE_ERROR("Gui::Builder::Add() failed! You can't add windows to node ID ({0}) because it was already split!", nodeID);
				return false;
			}

			// Override flags
			if (flags != WindowFlags_Unchanged)
			{
				window->flags = flags;
				window->imGuiClass.DockingAllowUnclassed = !(bool)(flags & WindowFlags_NoDockingUnclassed);
			}

			if (type != WindowType::ANY || forceType)
			{
				window->type = type;
				window->imGuiClass.ClassId = (ImGuiID)type;
			}

			// Add window to node and groups
			DockBuilderDockWindow(window->name, nodeID);

			// Update group flags as needed
			if (groups.find(nodeID) != groups.end())
			{
				auto[group_flags, window_group] = groups[nodeID];

				if ((bool)(window->flags & WindowFlags_AutoHideTab) && !(bool)(group_flags & WindowFlags_AutoHideTab))
				{
					// Don't warn about this, since having more than one docked window in a group will disable this anyway
					node->LocalFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
					group_flags |= WindowFlags_AutoHideTab;
				}

				if (// Window has close disabled, but group doesn't already have it
					((bool)(window->flags & (WindowFlags_DisableClose | WindowFlags_DisableCloseForce)) && !(bool)(group_flags & (WindowFlags_DisableClose | WindowFlags_DisableCloseForce)))
					||
					// Window does have close button, but group doesn't specify DisableCloseForce
					(!(bool)(window->flags & (WindowFlags_DisableClose | WindowFlags_DisableCloseForce)) && !(bool)(group_flags & WindowFlags_DisableCloseForce))
				)
				{
					// Log warnings, this might be unintentional!
					SURF_CORE_WARN("\
Window \"{0}\" added to group with Gui::Builder has inconsistent use of the flag\n\
`WindowFlags_DisableClose`, this option will take precedence over other windows in the group.\n\
Please consider using consistent flags on windows, or use `WindowFlags_DisableCloseForce`\n\
to supress this warning.", window->name);
					node->HasCloseButton = false;

					// Prevent further warnings
					group_flags |= WindowFlags_DisableCloseForce;
				}

				if (// Window has tab disabled, but group doesn't already have it
					((bool)(window->flags & (WindowFlags_DisableTab | WindowFlags_DisableTabForce)) && !(bool)(group_flags & (WindowFlags_DisableTab | WindowFlags_DisableTabForce)))
					||
					// Window does have tabs, but group doesn't specify DisableTabForce
					(!(bool)(window->flags & (WindowFlags_DisableTab | WindowFlags_DisableTabForce)) && !(bool)(group_flags & WindowFlags_DisableTabForce))
				)
				{
					// Log warnings, this might be unintentional!
					SURF_CORE_WARN("\
Window \"{0}\" added to group with Gui::Builder has inconsistent use of the flag\n\
`WindowFlags_DisableTab`, this option will take precedence over other windows in the group.\n\
Please consider using consistent flags on windows, or use `WindowFlags_DisableTabForce`\n\
to supress this warning.", window->name);
					node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

					// Prevent further warnings
					group_flags |= WindowFlags_DisableTabForce;
				}

				// Update window group
				window_group.push_back(window);
				groups[nodeID] = { group_flags, window_group };
			}
			else
			{
				// Add window to new docking group
				groups[nodeID] = { window->flags, {window} };

				node->LocalFlags |= (bool)(window->flags & WindowFlags_AutoHideTab) ? ImGuiDockNodeFlags_AutoHideTabBar : 0;
				node->LocalFlags |= (bool)(window->flags & (WindowFlags_DisableTab | WindowFlags_DisableTabForce)) ? ImGuiDockNodeFlags_NoTabBar : 0;
				
				if ((bool)(window->flags & (WindowFlags_DisableClose | WindowFlags_DisableCloseForce)))
					node->HasCloseButton = false;
			}
		}

		std::pair<ImGuiID, ImGuiID> Builder::Split(ImGuiDir direction, float ratio, const ImGuiID nodeID)
		{
			// Check that we are actually building
			if (!building)
			{
				SURF_CORE_WARN("Gui::Builder::Split() failed! Make sure you only call docking functions if BeginLayout() returns true!");
				return { 0, 0 };
			}

			ImGuiID given = (nodeID == 0) ? active_id : nodeID;

			bool problem = false; // Let's us print multiple errors at once, just in case.
			// Check that the node has no windows
			if (groups.find(given) != groups.end())
			{
				if (nodeID == 0)
					SURF_CORE_ERROR("Gui::Builder::Split() failed! Cannot split ACTIVE node with id ({0}) because windows have already been docked in it!", given);
				else
					SURF_CORE_ERROR("Gui::Builder::Split() failed! Cannot split node with id ({0}) because windows have already been docked in it!", given);
				problem = true;
			}

			// Check ratio is between 0 and 1, not inclusive
			if (ratio <= 0 || ratio >= 1)
			{
				SURF_CORE_ERROR("Gui::Builder::Split() failed! Specified ratio ({0}) must be between 0 and 1, not inclusive!", ratio);
				problem = true;
			}

			// Check that direction is valid
			if (direction < ImGuiDir_Left || direction > ImGuiDir_Down)
			{
				SURF_CORE_ERROR("Gui::Builder::Split() failed! Direction ({0}) is not valid, must be an integer between ({1}) and ({2}), please use the enums defined by ImGuiDir!", direction, ImGuiDir_Left, ImGuiDir_Down);
				problem = true;
			}

			if (problem) return { 0, 0 };

			ImGuiID split, other;
			active_id = split = DockBuilderSplitNode(given, direction, ratio, NULL, &other);
			
			if (direction == ImGuiDir_Left || direction == ImGuiDir_Right)
			{
				// Unset TOP or BOTTOM if it was just split
				if (given == dock_top)
					dock_top = 0;
				else if (given == dock_bottom)
					dock_bottom = 0;

				// Set LEFT and RIGHT
				if (direction == ImGuiDir_Left)
				{
					dock_left = split;
					dock_right = other;
				}
				else
				{
					dock_right = split;
					dock_left = other;
				}
			}
			else
			{
				// Unset LEFT or RIGHT if it was just split
				if (given == dock_left)
					dock_left = 0;
				else if (given == dock_right)
					dock_right = 0;

				// Set TOP and BOTTOM
				if (direction == ImGuiDir_Up)
				{
					dock_top = split;
					dock_bottom = other;
				}
				else
				{
					dock_bottom = split;
					dock_top = other;
				}
			}

			return { split, other };
		}
	}
}
