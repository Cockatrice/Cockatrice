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
        property real uLogoGlow: bannerConfig.logoGlow
        fragmentShader: "qrc:/onboarding/shaders/brand_banner.frag.qsb"
    }

    // The hero logo itself — breathes cleanly over a 0.5–1.0 opacity range
    Image {
        id: logo
        anchors.centerIn: parent
        visible: bannerConfig.logoVisible
        source: "qrc:/resources/cockatrice-logo-white.svg"
        width: root.height * 0.6
        height: width * (sourceSize.height > 0 ? sourceSize.height / Math.max(sourceSize.width, 1) : 1)
        fillMode: Image.PreserveAspectFit
        smooth: true
        opacity: 0.5 + 0.5 * bannerConfig.logoGlow
        sourceSize: Qt.size(256, 256)

        Behavior on opacity { NumberAnimation { duration: 300; easing.type: Easing.InOutSine } }

        transform: Scale {
            origin.x: logo.width / 2
            origin.y: logo.height / 2
            xScale: 0.94 + 0.06 * bannerConfig.logoGlow
            yScale: 0.94 + 0.06 * bannerConfig.logoGlow
        }
    }
}
