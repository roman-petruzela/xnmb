#include <ul/menu/ui/ui_WaveBackground.hpp>
#include <cmath>

namespace ul::menu::ui {

    namespace {

        struct BlobMotion {
            double speed_x;
            double speed_y;
            double phase;
            double radius_factor;
        };

        // Different speed/phase/size per blob so they drift independently instead of
        // moving as one rigid shape.
        constexpr BlobMotion Blobs[WaveBackground::BlobCount] = {
            { 0.07, 0.05, 0.0, 0.32 },
            { -0.05, 0.08, 1.7, 0.26 },
            { 0.09, -0.06, 3.1, 0.30 },
            { -0.08, -0.04, 4.6, 0.22 },
        };

    }

    WaveBackground::WaveBackground(const s32 x, const s32 y, const s32 w, const s32 h) :
        x(x), y(y), w(w), h(h),
        base_color(6, 109, 208, 0xFF),
        blob_color(0, 148, 255, 0x40),
        start_tp(std::chrono::steady_clock::now()) {}

    void WaveBackground::OnRender(pu::ui::render::Renderer::Ref &drawer, const s32 x, const s32 y) {
        drawer->RenderRectangleFill(this->base_color, x, y, this->w, this->h);

        const auto elapsed_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - this->start_tp).count();
        for(u32 i = 0; i < BlobCount; i++) {
            const auto &blob = Blobs[i];
            const auto radius = static_cast<s32>(this->h * blob.radius_factor);
            const auto center_x = x + (this->w / 2) + static_cast<s32>(std::sin(elapsed_s * blob.speed_x + blob.phase) * (this->w / 2.2));
            const auto center_y = y + (this->h / 2) + static_cast<s32>(std::cos(elapsed_s * blob.speed_y + blob.phase) * (this->h / 2.2));
            drawer->RenderCircleFill(this->blob_color, center_x, center_y, radius);
        }
    }

}
