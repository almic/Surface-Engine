#pragma once

#include "Core.h"
#include "Log.h"
#include "Event.h"

#include <string>
#include <vector>

namespace Surface {

	class Application;

	class SURF_API Layer
	{
		friend class View;
	public:
		Layer(const std::string& name) { this->name = name; }
		virtual ~Layer() {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnShow(bool& is_visible) { is_visible = true; }
		virtual void OnHide(bool& is_visible) { is_visible = false; }
		virtual void OnEvent(Event& event) {}

		virtual void Attach(Application* app)
		{
			this->app = app;
			OnAttach();
		}
		virtual void Detach() final { OnDetach(); }
		virtual void Update() final { OnUpdate(); }
		virtual void Show() final { OnShow(visible); }
		virtual void Hide() final { OnHide(visible); }
		virtual bool IsVisible() final { return visible; }
		virtual void SendEvent(Event& event) final { if (visible) OnEvent(event); }

		const char* GetName() const { return name.c_str(); }
		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << GetName() << " ";
			if (visible) ss << "visible";
			else ss << "hidden";
			return ss.str();
		}

	protected:
		Application* app = nullptr;
		bool visible = true;
		std::string name;
	};

	class SURF_API Overlay
	{
		friend class View;
	public:
		Overlay(const std::string& name) { this->name = name; }
		virtual ~Overlay() {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnShow(bool& is_visible) { is_visible = true; }
		virtual void OnHide(bool& is_visible) { is_visible = false; }
		virtual void OnEvent(Event& event) {}

		virtual void Attach(Application* app)
		{
			this->app = app;
			OnAttach();
		}
		virtual void Detach() final { OnDetach(); }
		virtual void Update() final { OnUpdate(); }
		virtual void Show() final { OnShow(visible); }
		virtual void Hide() final { OnHide(visible); }
		virtual bool IsVisible() final { return visible; }
		virtual void SendEvent(Surface::Event& event) final { OnEvent(event); }

		const char* GetName() const { return name.c_str(); }
		virtual std::string ToString() const
		{
			std::stringstream ss;
			ss << GetName() << " ";
			if (visible) ss << "visible";
			else ss << "hidden";
			return ss.str();
		}

	protected:
		Application* app = nullptr;
		bool visible = true;
		std::string name;
	};

	inline std::ostream& operator<<(std::ostream& os, const Layer& l) { return os << l.ToString(); }
	inline std::ostream& operator<<(std::ostream& os, const Overlay& l) { return os << l.ToString(); }

	typedef std::vector<Layer*> LayerVector;
	typedef std::vector<Overlay*> OverlayVector;


	class SURF_API View
	{
		friend class Application;
	public:
		View(Application* app, const std::string& name)
		{
			this->app = app;
			this->name = name;
		}
		~View() { Clear(); }

		void Clear() {
			for (Layer* layer : layers)
				delete layer;
			for (Overlay* overlay : overlays)
				delete overlay;
		}

		// Adds layers, which are rendered in order such that the first layer is above everything else.
		// No two layers may share the same name!! Returns true if the layer was added, false if
		// one with the same name already exists.
		bool AddLayer(Layer* layer) { return Add<Layer>(app, layer, layers); }

		// Adds overlays, which are rendered after all layers
		bool AddOverlay(Overlay* overlay) { return Add<Overlay>(app, overlay, overlays); }

		// Removes a layer, by name or pointer
		void RemoveLayer(Layer* layer) { Remove<Layer>(layer, layers); }
		void RemoveLayer(const std::string& name) { Remove<Layer>(name, layers); }
		void RemoveOverlay(Overlay* overlay) { Remove<Overlay>(overlay, overlays); }
		void RemoveOverlay(const std::string& name) { Remove<Overlay>(name, overlays); }

		// Retrieves a layer by name
		Layer* FindLayer(const std::string& name) { return Find<Layer>(name, layers); }
		Overlay* FindOverlay(const std::string& name) { return Find<Overlay>(name, overlays); }

	private:
		Application* app = nullptr;
		std::string name;
		LayerVector layers;
		OverlayVector overlays;

		template<typename Layer> static inline bool Add(Application* app, Layer* layer, std::vector<Layer*>& lvec)
		{
			// Ensure layer with same name doesn't exist
			for (Layer* other : lvec)
			{
				if (other == layer || other->name == layer->name)
					return false;
			}
			lvec.push_back(layer);
			layer->Attach(app);
			return true;
		}

		template<typename Layer> static inline void Remove(Layer* layer, std::vector<Layer*>& lvec)
		{
			auto it = std::find(lvec.begin(), lvec.end(), layer);
			if (it != lvec.end())
			{
				lvec.erase(it);
				layer->Detach();
			}
		}

		template<typename Layer> static inline void Remove(const std::string& name, std::vector<Layer*>& lvec)
		{
			std::vector<Layer*>::iterator index = lvec.begin();
			for (Layer* layer : lvec)
			{
				if (layer->name == name)
				{
					lvec.erase(index);
					layer->Detach();
					return;
				}
				if (++index == lvec.end()) return;
			}
		}

		template<typename Layer> static inline Layer* Find(const std::string& name, std::vector<Layer*>& lvec)
		{
			for (Layer* layer : lvec)
			{
				if (layer->name == name)
				{
					return layer;
				}
			}
			return nullptr;
		}
	};

}
