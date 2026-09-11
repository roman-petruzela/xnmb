#include <ul/menu/ui/ui_XmbList.hpp>
#include <cmath>

namespace ul::menu::ui {

    XmbList::XmbList(const s32 x, const s32 y, const s32 w, const s32 h, EntryActivatedCallback activated_cb, FocusedEntryChangedCallback focus_changed_cb) :
        x(x), y(y), w(w), h(h), enabled(true), focused_idx(0), activated_cb(activated_cb), focus_changed_cb(focus_changed_cb) {}

    void XmbList::EnsureIconLoaded(const u32 idx) {
        if((idx < this->entry_icons.size()) && (this->entry_icons.at(idx) == nullptr)) {
            this->entry_icons.at(idx) = LoadEntryIconTexture(this->entries.at(idx));
        }
    }

    void XmbList::LoadPath(const std::string &path, const u32 focus_idx) {
        this->cur_path = path;
        this->entries = LoadEntries(path);
        this->entry_icons.assign(this->entries.size(), nullptr);
        this->focused_idx = this->entries.empty() ? 0 : std::min(focus_idx, static_cast<u32>(this->entries.size() - 1));

        if(this->focus_changed_cb) {
            this->focus_changed_cb();
        }
    }

    void XmbList::Reload() {
        this->LoadPath(this->cur_path, this->focused_idx);
    }

    bool XmbList::IsInRoot() const {
        return this->cur_path.length() == GetActiveMenuPath().length();
    }

    void XmbList::OnRender(pu::ui::render::Renderer::Ref &drawer, const s32 x, const s32 y) {
        if(this->entries.empty()) {
            return;
        }

        const auto center_x = x + (this->w / 2);
        const auto center_y = y + (this->h / 2);

        for(s32 offset = -static_cast<s32>(VisibleSideCount); offset <= static_cast<s32>(VisibleSideCount); offset++) {
            const auto idx = static_cast<s32>(this->focused_idx) + offset;
            if((idx < 0) || (idx >= static_cast<s32>(this->entries.size()))) {
                continue;
            }

            this->EnsureIconLoaded(static_cast<u32>(idx));
            const auto &icon = this->entry_icons.at(static_cast<u32>(idx));
            if(icon == nullptr) {
                continue;
            }

            // 0 at the focused item, 1 at the furthest visible neighbour: drives both the
            // shrink and the fade, giving the classic XMB "receding" look.
            const auto dist = std::abs(offset);
            const auto t = static_cast<double>(dist) / static_cast<double>(VisibleSideCount + 1);
            const auto size = static_cast<s32>(FocusedIconSize - t * (FocusedIconSize - MinIconSize));
            const auto alpha = static_cast<u8>(255.0 - t * 180.0);

            const auto item_x = center_x - (size / 2);
            const auto item_y = center_y + offset * static_cast<s32>(ItemSpacing) - (size / 2);

            drawer->RenderTexture(icon->Get(), item_x, item_y, pu::ui::render::TextureRenderOptions(alpha, size, size, {}, {}, {}));
        }
    }

    void XmbList::OnInput(const u64 keys_down, const u64 keys_up, const u64 keys_held, const pu::ui::TouchPoint touch_pos) {
        if(!this->enabled || this->entries.empty()) {
            return;
        }

        if(keys_down & HidNpadButton_Up) {
            if(this->focused_idx > 0) {
                this->focused_idx--;
                if(this->focus_changed_cb) {
                    this->focus_changed_cb();
                }
            }
        }
        else if(keys_down & HidNpadButton_Down) {
            if((this->focused_idx + 1) < this->entries.size()) {
                this->focused_idx++;
                if(this->focus_changed_cb) {
                    this->focus_changed_cb();
                }
            }
        }
        else if(keys_down & HidNpadButton_A) {
            if(this->activated_cb) {
                this->activated_cb(this->entries.at(this->focused_idx));
            }
        }
    }

}
