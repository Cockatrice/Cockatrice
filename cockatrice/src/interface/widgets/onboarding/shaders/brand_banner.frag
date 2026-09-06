#version 440

// ════════════════════════════════════════════════════════════════════════
// brand_banner.frag
//
// One shader, six motifs (uMode 0..5).  All motifs composite over a shared
// backgroundField() whose colour is flow-noise-modulated blend of uColorA
// and uColorB.  SDFs operate in aspect-corrected space (ac.x = uv.x *
// uAspect) to preserve shape proportions on the wide banner.
//
// IMPORTANT: the uniform block below must list custom uniforms in EXACTLY
// the order they're declared as properties on each ShaderEffect instance in
// BrandBanner.qml (after the two Qt-supplied members, qt_Matrix/qt_Opacity).
// ════════════════════════════════════════════════════════════════════════

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    float iTime;
    float uAspect;
    float uMode;
    float uSpeed;
    float uSeed;
    vec4 uColorA;
    vec4 uColorB;
    vec4 uAccent;
    float uLogoGlow;
};

// ── Primitives ──────────────────────────────────────────────────────────

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 3; i++) {
        v += amp * valueNoise(p);
        p *= 2.03;
        amp *= 0.5;
    }
    return v;
}

float flowNoise(vec2 p, float t)
{
    vec2 warp1 = vec2(fbm(p + vec2(0.0, 0.0)), fbm(p + vec2(5.2, 1.3)));
    vec2 warp2 = vec2(fbm(p + 4.0 * warp1 + vec2(1.7, 9.2) + t * 0.6),
                      fbm(p + 4.0 * warp1 + vec2(8.3, 2.8) - t * 0.5));
    return fbm(p + 4.0 * warp2 + t * 0.15);
}

float bloom(float d, float coreRadius, float haloRadius)
{
    float core = exp(-(d * d) / (coreRadius * coreRadius));
    float halo = exp(-d / haloRadius) * 0.35;
    return core + halo;
}

float roundedBoxSDF(vec2 p, vec2 halfSize, float radius)
{
    vec2 d = abs(p) - halfSize + radius;
    return length(max(d, 0.0)) - radius + min(max(d.x, d.y), 0.0);
}

// Rotated box SDF -- applies 2D rotation to p before evaluating roundedBoxSDF.
float rotatedBoxSDF(vec2 p, vec2 halfSize, float radius, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    vec2 rp = vec2(p.x * c - p.y * s, p.x * s + p.y * c);
    return roundedBoxSDF(rp, halfSize, radius);
}

float vignette(vec2 uv)
{
    vec2 c = uv - 0.5;
    c.x *= max(uAspect, 0.0001);
    return smoothstep(1.0, 0.25, length(c));
}

// ── Shared background ───────────────────────────────────────────────────

vec3 backgroundField(vec2 uv, float time)
{
    // Diagonal luminance gradient from (0,0) to (1,1) used as blend factor
    // between uColorA and uColorB; modulated by flowNoise.
    float baseD = smoothstep(0.0, 1.0, uv.y * 0.5 + uv.x * 0.2);
    float painted = flowNoise(uv * 1.5, time * 0.04) - 0.5;
    baseD = clamp(baseD + painted * 0.12, 0.0, 1.0);

    vec3 col = mix(uColorA.rgb, uColorB.rgb, baseD);

    // Low-frequency fBM noise pushes local colour toward uColorB for depth
    float deep = fbm(uv * 1.0 + vec2(37.1, 12.4) + time * 0.015);
    col = mix(col, uColorB.rgb, (deep - 0.5) * 0.08);

    // Accent-coloured fog layer: flowNoise peaks above 0.6 contribute accent
    float fog = flowNoise(uv * 0.8 + vec2(100.0, 50.0), time * 0.02);
    col += uAccent.rgb * max(fog - 0.6, 0.0) * 0.10;

    return col;
}

// ── Motifs ──────────────────────────────────────────────────────────────

// Centre bloom, flow-noise shimmer gated to centre, and 48 orbiting ember
// particles that deflect into a tight ring near the centre.
vec3 motifWelcome(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;
    float asp = max(uAspect, 0.001);
    vec2 ac = vec2(uv.x * asp, uv.y);
    vec2 center = vec2(asp * 0.5, 0.5);
    float cDist = length(ac - center);

    // Centre bloom at logo position; intensity scales with uLogoGlow
    float centreLight = bloom(cDist, 0.08 * asp, 0.40 * asp);
    col += centreLight * 0.20 * uLogoGlow;

    // Flow-noise shimmer gated by Gaussian mask at centre
    float shimmer = flowNoise(ac * 0.8 + vec2(55.0, 33.0), t * 0.05) * 0.5 + 0.5;
    float shimmerMask = exp(-(cDist * cDist) / (0.18 * asp * 0.18 * asp));
    col += shimmer * shimmerMask * 0.04 * uLogoGlow;

    // 48 ember particles: hash-seeded position, speed, size, brightness.
    // Embers within a distance threshold of centre are deflected into an
    // orbital ring via tangent displacement perpendicular to the centre vector.
    const int EMBERS = 48;
    for (int i = 0; i < EMBERS; i++) {
        float fi = float(i);

        float baseX = hash21(vec2(fi * 7.31 + uSeed, fi * 3.17));
        float baseY = hash21(vec2(fi * 11.9 + uSeed * 1.4, fi * 5.53));

        float riseSpeed = 0.025 + hash21(vec2(fi * 1.7, uSeed * 2.1)) * 0.035;
        float driftAmp = 0.04 + hash21(vec2(fi * 9.3, uSeed)) * 0.06;
        float driftFreq = 0.3 + hash21(vec2(fi * 4.1, uSeed * 3.3)) * 0.5;

        float pX = baseX * asp + sin(t * driftFreq + fi * 1.7) * driftAmp * asp;
        float pY = fract(baseY + t * riseSpeed);

        float size = 0.006 + hash21(vec2(fi * 2.9, uSeed * 4.7)) * 0.012;
        float bright = 0.15 + hash21(vec2(fi * 6.1, uSeed * 0.9)) * 0.30;

        // Fade out near top/bottom edges
        float edgeFade = smoothstep(0.0, 0.12, pY) * smoothstep(1.0, 0.88, pY);
        float twinkle = 0.6 + 0.4 * sin(t * (1.2 + fi * 0.37) + fi * 2.9);

        vec2 ePos = vec2(pX, pY);

        // Embers near centre: deflect into orbital ring via tangent displacement
        vec2 toCenter = ePos - center;
        float distToCenter = length(toCenter);
        float ringWeight = smoothstep(0.38 * asp, 0.06 * asp, distToCenter);

        float orbitPhase = t * (0.15 + fi * 0.020) + fi * 2.3;
        float orbitAmount = 0.020 + hash21(vec2(fi * 12.3, uSeed * 2.7)) * 0.020;
        vec2 tangent = vec2(-toCenter.y, toCenter.x);
        vec2 deflected = ePos + tangent * ringWeight * orbitAmount * asp * sin(orbitPhase);

        float pushOut = ringWeight * (0.008 + hash21(vec2(fi * 6.7, uSeed * 1.1)) * 0.012) * asp;
        deflected += normalize(toCenter + 0.001) * pushOut;

        float dist = length(ac - deflected);
        float intensity = bright * edgeFade * twinkle;
        col += uAccent.rgb * bloom(dist, size, size * 4.0) * intensity;
    }

    return col;
}

// 25 card-shaped box SDFs at parallax depths drifting horizontally across
// the banner; each card has a semi-transparent fill, accent outline, and
// card-back diamond pattern.
vec3 motifCardDatabase(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;
    float asp = max(uAspect, 0.001);
    vec2 ac = vec2(uv.x * asp, uv.y);

    const int CARDS = 25;
    for (int i = 0; i < CARDS; i++) {
        float fi = float(i);

        // Parallax depth via hash; used to scale size, speed, brightness
        float depth = hash21(vec2(fi * 1.37 + uSeed, fi * 0.91));

        // Card dimensions in corrected space (portrait: height > width)
        float cardH = mix(0.055, 0.15, depth);
        cardH *= 0.85 + 0.30 * hash21(vec2(fi * 3.14, uSeed * 2.71));
        float cardW = cardH * 0.71; // 5:7 ratio

        // Horizontal drift; nearer cards (higher depth) move faster
        float speed = mix(0.06, 0.18, depth);
        float xPhase = hash21(vec2(fi * 7.13, uSeed * 4.37));
        xPhase = fract(xPhase + t * speed);
        float x = mix(-1.5, asp + 1.5, xPhase);

        // Vertical position: hash distribution with sinusoidal oscillation
        float yBase = hash21(vec2(fi * 2.91, uSeed * 1.63));
        float y = yBase + sin(t * 0.6 + fi * 1.9) * 0.035;
        y = clamp(y, cardH + 0.02, 1.0 - cardH - 0.02);

        // Random rotation angle ±4 degrees
        float tilt = (hash21(vec2(fi * 5.71, uSeed * 8.29)) - 0.5) * 0.14;

        vec2 p = ac - vec2(x, y);
        float d = rotatedBoxSDF(p, vec2(cardW, cardH), cardW * 0.14, tilt);

        // Semi-transparent dark fill
        float fill = smoothstep(0.015, -0.005, d);
        col = mix(col, uColorB.rgb * 0.55, fill * 0.50);

        // Accent outline
        float edge = smoothstep(0.035, 0.0, abs(d));
        col += uAccent.rgb * edge * mix(0.18, 0.50, 1.0 - depth);

        // Card-back diamond: smaller rotated box inset from card edges
        float innerD = rotatedBoxSDF(p, vec2(cardW * 0.45, cardH * 0.55), cardW * 0.08, tilt);
        float innerEdge = smoothstep(0.012, 0.0, abs(innerD));
        col += uAccent.rgb * innerEdge * fill * 0.12 * (1.0 - depth);

        // Centre dot
        float dotDist = length(p);
        col += uAccent.rgb * bloom(dotDist, 0.008, 0.02) * fill * 0.15 * (1.0 - depth);
    }
    return col;
}

// 4 horizontal bands with multi-frequency sinusoidal warp and pulsing width.
vec3 motifTheming(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;

    const int BANDS = 4;
    for (int i = 0; i < BANDS; i++) {
        float fi = float(i);
        float yCenter = 0.18 + fi * 0.22;

        // Three summed sinusoids for horizontal undulation
        float wave = sin(uv.x * 3.2 + t * 0.5 + fi * 2.1) * 0.08;
        wave += sin(uv.x * 7.0 - t * 0.3 + fi * 1.3) * 0.035;
        wave += sin(uv.x * 1.6 + t * 0.18 + fi * 3.7) * 0.05;

        float bandDist = abs(uv.y - yCenter - wave);
        float bandWidth = 0.04 + sin(t * 0.2 + fi * 0.8) * 0.012;
        float band = smoothstep(bandWidth, 0.0, bandDist);

        // Upper bands have higher intensity
        float intensity = mix(0.15, 0.38, 1.0 - fi / float(BANDS));
        col += uAccent.rgb * band * intensity;
    }

    return col;
}

// 14 nodes at pseudo-random positions with sinusoidal pulse; edges drawn
// between nodes within a threshold distance; central glow + periodic ring.
vec3 motifAccount(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;
    float asp = max(uAspect, 0.001);
    vec2 ac = vec2(uv.x * asp, uv.y);
    vec2 center = vec2(asp * 0.5, 0.5);

    const int NODES = 14;
    vec2 nodePos[14];
    float nodePulse[14];

    for (int i = 0; i < NODES; i++) {
        float fi = float(i);

        // Hash-seeded position with gentle sinusoidal drift
        float nx = hash21(vec2(fi * 3.17 + uSeed, fi * 1.93)) * asp;
        float ny = hash21(vec2(fi * 5.41 + uSeed * 1.7, fi * 2.79));

        float dx = sin(t * 0.12 + fi * 1.7) * 0.08;
        float dy = cos(t * 0.09 + fi * 2.3) * 0.04;
        vec2 pos = vec2(nx + dx, ny + dy);
        nodePos[i] = pos;

        // Per-node pulse phase, normalised to [0, 1]
        float pulsePhase = hash21(vec2(fi * 4.31, uSeed * 6.17));
        float pulse = sin(t * 0.8 + pulsePhase * 6.283) * 0.5 + 0.5;
        nodePulse[i] = pulse;

        // Node glow via bloom; intensity modulated by pulse
        float dist = length(ac - pos);
        col += uAccent.rgb * bloom(dist, 0.018, 0.08) * mix(0.20, 0.45, pulse);
    }

    // Edges: connect nodes within a radius threshold
    float connectDist = asp * 0.22;
    for (int i = 0; i < NODES; i++) {
        for (int j = i + 1; j < NODES; j++) {
            float pairDist = length(nodePos[i] - nodePos[j]);
            if (pairDist < connectDist) {
                float strength = 1.0 - pairDist / connectDist;
                vec2 pa = ac - nodePos[i];
                vec2 ba = nodePos[j] - nodePos[i];
                float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
                float lineDist = length(pa - ba * h);
                col += uAccent.rgb * smoothstep(0.010, 0.0, lineDist) * strength * 0.10;
            }
        }
    }

    // Central bloom at banner centre
    float cDist = length(ac - center);
    col += uAccent.rgb * bloom(cDist, 0.04, 0.25) * 0.12;

    // Periodic expanding ring from centre
    float ripplePhase = t * 0.4;
    float rippleDist = abs(cDist - fract(ripplePhase) * asp * 0.7);
    col += uAccent.rgb * smoothstep(0.02, 0.0, rippleDist) * 0.10;

    return col;
}

// 18x5 toggle-grid of rounded boxes with hash-driven on/off per cell;
// a scanning highlight sweeps L-to-R, brightening cells near the scan line.
vec3 motifPreferences(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;

    float cols = 18.0;
    float rows = 5.0;
    vec2 gridUV = uv * vec2(cols, rows);
    vec2 cell = fract(gridUV) - 0.5;
    vec2 cellId = floor(gridUV);

    // On/off state per cell, hash-seeded for pseudo-randomness
    float on = step(0.55, hash21(cellId + uSeed * 10.0));

    float d = roundedBoxSDF(cell, vec2(0.28, 0.32), 0.06);

    // Filled "on" cells
    float cellFill = smoothstep(0.04, -0.02, d);
    col += uAccent.rgb * cellFill * on * 0.18;

    // Cell borders (drawn on all cells)
    float border = smoothstep(0.025, 0.0, abs(d));
    col += uAccent.rgb * border * 0.06;

    // Scanning highlight: thin line + soft glow sweeping L-to-R
    float scanX = fract(t * 0.15);
    float scanDist = abs(uv.x - scanX);
    float scanLine = smoothstep(0.015, 0.0, scanDist);
    col += uAccent.rgb * scanLine * 0.40;

    float scanGlow = smoothstep(0.08, 0.0, scanDist);
    col += uAccent.rgb * scanGlow * 0.08;

    // "On" cells near the scan line get extra brightness
    float scanProximity = smoothstep(0.12, 0.0, scanDist);
    col += uAccent.rgb * cellFill * on * scanProximity * 0.15;

    return col;
}

// Centre radial bloom with sinusoidal pulse, 4 expanding ring halos with
// outer glow falloff, and 35 rising particles.
vec3 motifFinish(vec2 uv, vec3 bg, float t)
{
    vec3 col = bg;
    float asp = max(uAspect, 0.001);
    vec2 ac = vec2(uv.x * asp, uv.y);
    vec2 center = vec2(asp * 0.5, 0.5);
    float cDist = length(ac - center);

    // Centre bloom with sinusoidal pulse modulation
    float pulse = 0.65 + 0.35 * sin(t * 0.4);
    col += uAccent.rgb * bloom(cDist, 0.12, 0.55) * 0.10 * pulse;

    // 4 expanding rings: radius increases via phase; ring width grows with
    // expansion; combined with exponential outer glow falloff
    for (int i = 0; i < 4; i++) {
        float fi = float(i);
        float phase = fract(t * 0.06 + fi * 0.25);
        float ringRadius = phase * asp * 0.7;
        float ringDist = abs(cDist - ringRadius);
        float ringWidth = 0.025 + phase * 0.025;
        float ring = smoothstep(ringWidth, 0.0, ringDist);
        float outerGlow = exp(-ringDist / (0.03 + phase * 0.02)) * 0.3;
        float combined = ring + outerGlow;
        float fade = 1.0 - phase * 0.5;
        col += uAccent.rgb * combined * fade * 0.15;
    }

    // 35 particles rising vertically with sinusoidal horizontal drift;
    // each particle uses bloom with edge fade and twinkle animation
    const int PARTICLES = 35;
    for (int i = 0; i < PARTICLES; i++) {
        float fi = float(i);
        float baseX = hash21(vec2(fi * 13.7 + uSeed, fi * 7.31));
        float baseY = hash21(vec2(fi * 23.1 + uSeed * 1.9, fi * 11.3));

        float riseSpeed = 0.04 + hash21(vec2(fi * 3.1, uSeed * 2.7)) * 0.06;
        float driftAmp = 0.03 + hash21(vec2(fi * 8.9, uSeed)) * 0.05;
        float driftFreq = 0.4 + hash21(vec2(fi * 5.3, uSeed * 4.1)) * 0.6;

        float pX = baseX * asp + sin(t * driftFreq + fi * 2.3) * driftAmp * asp;
        float pY = fract(baseY + t * riseSpeed);

        float size = 0.005 + hash21(vec2(fi * 4.7, uSeed * 3.9)) * 0.010;
        float bright = 0.12 + hash21(vec2(fi * 7.1, uSeed * 1.3)) * 0.25;

        float edgeFade = smoothstep(0.0, 0.1, pY) * smoothstep(1.0, 0.9, pY);
        float twinkle = 0.5 + 0.5 * sin(t * (1.8 + fi * 0.43) + fi * 3.1);

        vec2 pPos = vec2(pX, pY);
        float dist = length(ac - pPos);
        col += uAccent.rgb * bloom(dist, size, size * 3.5) * bright * edgeFade * twinkle;
    }

    return col;
}

// ── Main ────────────────────────────────────────────────────────────────

void main()
{
    vec2 uv = qt_TexCoord0;
    float t = iTime * uSpeed;

    vec3 bg = backgroundField(uv, iTime);

    vec3 col;
    if (uMode < 0.5) col = motifWelcome(uv, bg, t);
    else if (uMode < 1.5) col = motifCardDatabase(uv, bg, t);
    else if (uMode < 2.5) col = motifTheming(uv, bg, t);
    else if (uMode < 3.5) col = motifAccount(uv, bg, t);
    else if (uMode < 4.5) col = motifPreferences(uv, bg, t);
    else col = motifFinish(uv, bg, t);

    col *= mix(0.62, 1.0, vignette(uv));
    fragColor = vec4(col, 1.0) * qt_Opacity;
}
