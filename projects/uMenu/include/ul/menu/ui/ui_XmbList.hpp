
#pragma once
#include <ul/menu/menu_Entries.hpp>
#include <ul/menu/ui/ui_Common.hpp>
#include <pu/Plutonium>

namespace ul::menu::ui {

    // A single vertical column of entries, PSP XMB style: the focused entry sits centered and
    // full-size, neighbours above/below shrink and fade out with distance. Renders exactly one
    // category's contents (e.g. the Games list) - switching categories is handled by whichever
    // layout owns this element (see MainMenuLayout), which just calls LoadPath() again.
    class XmbList : public pu::ui::elm::Element {
        public:
            static constexpr u32 FocusedIconSize = 200;
            static constexpr u32 MinIconSize = 64;
            static constexpr u32 ItemSpacing = 130;
            static constexpr u32 VisibleSideCount = 3;

            using EntryActivatedCallback = std::function<void(Entry &)>;
            // Called whenever the focused index changes, so the owner can refresh its own
            // name/author text blocks etc. (mirrors uLaunch's existing cur_entry_main_text pattern).
            using FocusedEntryChangedCallback = std::function<void()>;

        private:
            s32 x;
            s32 y;
            s32 w;
            s32 h;
            bool enabled;
            std::string cur_path;
            std::vector<Entry> entries;
            std::vector<pu::sdl2::TextureHandle::Ref> entry_icons;
            u32 focused_idx;
            EntryActivatedCallback activated_cb;
            FocusedEntryChangedCallback focus_changed_cb;

            void EnsureIconLoaded(const u32 idx);

        public:
            XmbList(const s32 x, const s32 y, const s32 w, const s32 h, EntryActivatedCallback activated_cb, FocusedEntryChangedCallback focus_changed_cb);
            PU_SMART_CTOR(XmbList)

            inline s32 GetX() override {
                return this->x;
            }

            inline void SetX(const s32 x) {
                this->x = x;
            }

            inline s32 GetY() override {
                return this->y;
            }

            inline void SetY(const s32 y) {
                this->y = y;
            }

            inline s32 GetWidth() override {
                return this->w;
            }

            inline void SetWidth(const s32 w) {
                this->w = w;
            }

            inline s32 GetHeight() override {
                return this->h;
            }

            inline void SetHeight(const s32 h) {
                this->h = h;
            }

            inline void SetEnabled(const bool enabled) {
                this->enabled = enabled;
            }

            // focus_idx: which entry to focus once loaded (clamped to the loaded entry count)
            void LoadPath(const std::string &path, const u32 focus_idx = 0);

            // Re-reads the current path from disk (e.g. after creating/removing/renaming an entry)
            void Reload();

            inline bool IsEmpty() const {
                return this->entries.empty();
            }

            inline bool HasFocusedEntry() const {
                return this->focused_idx < this->entries.size();
            }

            inline Entry &GetFocusedEntry() {
                return this->entries.at(this->focused_idx);
            }

            inline std::vector<Entry> &GetEntries() {
                return this->entries;
            }

            inline const std::string &GetPath() const {
                return this->cur_path;
            }

            inline u32 GetFocusedIndex() const {
                return this->focused_idx;
            }

            // Resets focus back to the first entry (e.g. on HOME button press)
            inline void Rewind() {
                if(!this->entries.empty()) {
                    this->focused_idx = 0;
                    if(this->focus_changed_cb) {
                        this->focus_changed_cb();
                    }
                }
            }

            bool IsInRoot() const;

            void OnRender(pu::ui::render::Renderer::Ref &drawer, const s32 x, const s32 y) override;
            void OnInput(const u64 keys_down, const u64 keys_up, const u64 keys_held, const pu::ui::TouchPoint touch_pos) override;
    };

}
