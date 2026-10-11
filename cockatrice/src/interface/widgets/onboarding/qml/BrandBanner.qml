import QtQuick

Item {
    id: root

    ShaderEffect {
        id: effectA
        anchors.fill: parent
        opacity: bannerConfig.frontIsA ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 450; easing.type: Easing.InOutCubic } }
        property real iTime: bannerConfig.time
        property real uAspect: bannerConfig.aspect
        property real uMode: bannerConfig.modeA
        property real uSpeed: bannerConfig.speedA
        property real uSeed: bannerConfig.seedA
        property vector4d uColorA: Qt.vector4d(bannerConfig.colorA.r, bannerConfig.colorA.g, bannerConfig.colorA.b, 1.0)
        property vector4d uColorB: Qt.vector4d(bannerConfig.colorB.r, bannerConfig.colorB.g, bannerConfig.colorB.b, 1.0)
        property vector4d uAccent: Qt.vector4d(bannerConfig.accent.r, bannerConfig.accent.g, bannerConfig.accent.b, 1.0)
        property vector4d uGlowColor: Qt.vector4d(bannerConfig.glowColor.r, bannerConfig.glowColor.g, bannerConfig.glowColor.b, 1.0)
        property real uVignetteMin: bannerConfig.vignetteMin
        property real uLogoGlow: bannerConfig.logoGlow
        fragmentShader: "qrc:/onboarding/shaders/brand_banner.frag.qsb"
    }

    ShaderEffect {
        id: effectB
        anchors.fill: parent
        opacity: bannerConfig.frontIsA ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: 450; easing.type: Easing.InOutCubic } }
        property real iTime: bannerConfig.time
        property real uAspect: bannerConfig.aspect
        property real uMode: bannerConfig.modeB
        property real uSpeed: bannerConfig.speedB
        property real uSeed: bannerConfig.seedB
        property vector4d uColorA: Qt.vector4d(bannerConfig.colorA.r, bannerConfig.colorA.g, bannerConfig.colorA.b, 1.0)
        property vector4d uColorB: Qt.vector4d(bannerConfig.colorB.r, bannerConfig.colorB.g, bannerConfig.colorB.b, 1.0)
        property vector4d uAccent: Qt.vector4d(bannerConfig.accent.r, bannerConfig.accent.g, bannerConfig.accent.b, 1.0)
        property vector4d uGlowColor: Qt.vector4d(bannerConfig.glowColor.r, bannerConfig.glowColor.g, bannerConfig.glowColor.b, 1.0)
        property real uVignetteMin: bannerConfig.vignetteMin
        property real uLogoGlow: bannerConfig.logoGlow
        fragmentShader: "qrc:/onboarding/shaders/brand_banner.frag.qsb"
    }

    // The white logo sits at full opacity on top of the static gradient plate —
    // no glow, no breathing. The plate matches home_widget's QPainter composite.
    Item {
        id: logoHost
        visible: bannerConfig.logoVisible
        anchors.centerIn: parent
        width: root.height * 0.6
        height: width

        // The full-color logo renders beneath the white mark and is consumed as
        // a texture (layer.enabled) by the plate shader's silhouette mask, so
        // the gradient is clipped to the bird exactly as the SVG's gradient
        // paths are. It is never drawn to the screen itself.
        Image {
            id: silhouetteMask
            anchors.fill: parent
            source: "qrc:/resources/cockatrice.svg"
            sourceSize: Qt.size(256, 256)
            fillMode: Image.PreserveAspectFit
            smooth: true
            visible: false
            layer.enabled: true
            layer.smooth: true
        }

        // The logo's gradient plate, drawn behind the mark: a linear
        // AccentSoft (light) -> AccentStrong (dark) sheet along the same
        // top-left -> bottom-right userSpaceOnUse axis the baked-in SVG used,
        // clipped to the bird silhouette via uSilhouette. The white highlight
        // path above is theme independent. Sized to the logo itself — no
        // rounded badge, matching home_widget's QPainter composite. Static.
        // Small ShaderEffect, Qt 6.4-safe.
        ShaderEffect {
            id: brandPlate
            anchors.fill: parent
            property vector4d uStrong: Qt.vector4d(bannerConfig.brandStrong.r, bannerConfig.brandStrong.g,
                                                   bannerConfig.brandStrong.b, 1.0)
            property vector4d uSoft: Qt.vector4d(bannerConfig.brandSoft.r, bannerConfig.brandSoft.g,
                                                 bannerConfig.brandSoft.b, 1.0)
            property var uSilhouette: silhouetteMask
            fragmentShader: "qrc:/onboarding/shaders/brand_plate.frag.qsb"
        }

        Image {
            id: logoImage
            anchors.fill: parent
            source: "qrc:/resources/cockatrice-logo-white.svg"
            sourceSize: Qt.size(256, 256)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
    }
}