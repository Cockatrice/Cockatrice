#version 440

// The logo's gradient plate, drawn by us rather than the baked-in SVG: a
// linear blend between the two brand appColors (light AccentSoft at the
// top-left grading to dark AccentStrong at the bottom-right, mirroring
// cockatrice.svg's linearGradient4265-7-8 userSpaceOnUse axis), clipped to
// the bird's full silhouette via the full-color logo's alpha (uSilhouette).
// The white highlight path (cockatrice-logo-white) is overlaid in QML on top,
// exactly as the SVG stacks its white path over the gradient paths. Fully
// static: no glow, no breathing — the plate just sits there like the home
// widget's QPainter composite.

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 uStrong;
    vec4 uSoft;
};

// The full-color logo's alpha channel acts as the silhouette mask: the
// gradient only appears inside the bird, exactly like the SVG's gradient paths.
layout(binding = 1) uniform sampler2D uSilhouette;

void main()
{
    // Recreate cockatrice.svg's own gradient geometry (linearGradient4265-7-8,
    // userSpaceOnUse): light AccentSoft at the start point S=(-8.097,-97.746),
    // dark AccentStrong at the end E=(162.455,295.208), on the SVG's 300x300
    // canvas. Normalized to UV space, V=E-S=(0.5685,1.3098), so
    // t = dot(uv - S_norm, V)/|V|^2 with S_norm=(-0.0270,-0.3258).
    float t = clamp(dot(qt_TexCoord0 - vec2(-0.02699, -0.32582), vec2(0.56851, 1.30985)) / 2.03891, 0.0, 1.0);
    vec3 color = mix(uSoft.rgb, uStrong.rgb, t);

    // Anti-aliased silhouette clip from the full-color logo's alpha.
    float alpha = texture(uSilhouette, qt_TexCoord0).a;

    fragColor = vec4(color * alpha, alpha) * qt_Opacity;
}