
#pragma once
#include <pu/Plutonium>
#include <chrono>

namespace ul::menu::ui {

    // Animated PSP XMB-style background: a solid base color with a few large, softly
    // colored blobs drifting around over time. Drawn as plain shape fills (no image
    // assets needed), so it works before any dedicated "psp-xmb" theme art exists.
    class WaveBackground : public pu::ui::elm::Element {
        public:
            static constexpr u32 BlobCount = 4;

        private:
            s32 x;
            s32 y;
            s32 w;
            s32 h;
            pu::ui::Color base_color;
            pu::ui::Color blob_color;
            std::chrono::steady_clock::time_point start_tp;

        public:
            WaveBackground(const s32 x, const s32 y, const s32 w, const s32 h);
            PU_SMART_CTOR(WaveBackground)

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

            // Lets the current category (Games/Themes/Settings/...) tint the background.
            inline void SetColors(const pu::ui::Color base, const pu::ui::Color blob) {
                this->base_color = base;
                this->blob_color = blob;
            }

            void OnRender(pu::ui::render::Renderer::Ref &drawer, const s32 x, const s32 y) override;
            void OnInput(const u64 keys_down, const u64 keys_up, const u64 keys_held, const pu::ui::TouchPoint touch_pos) override {}
    };

}
