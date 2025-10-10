import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property var router

    // 🔹 Если ты регистрируешь в C++ как Auth — оставь как есть.
    // Если как LoginCtl — замени Auth на LoginCtl.
    readonly property var loginCtl: (typeof Auth !== "undefined") ? Auth : null
    readonly property var _themeObj: (typeof AppTheme !== "undefined" && AppTheme) ? AppTheme : null

    readonly property real s: (_themeObj && _themeObj.fontScale) ? _themeObj.fontScale : 1.0
    function sp(px){ return Math.max(1, Math.round(px*s)) }

    readonly property var t: ({
        fontBody: sp(16), fontH1: sp(22),
        controlH: sp(48), fieldH: sp(44), corner: sp(10),
        spacing: sp(12), margin: sp(16),
        border:   (_themeObj && _themeObj.border)     ? _themeObj.border     : "#333333",
        bg:       (_themeObj && _themeObj.bgColor)    ? _themeObj.bgColor    : "#111827",
        panelBg:  (_themeObj && _themeObj.panelBg)    ? _themeObj.panelBg    : "#1F2937",
        text:     (_themeObj && _themeObj.textColor)  ? _themeObj.textColor  : "#F9FAFB",
        fieldBg:  (_themeObj && _themeObj.panelBg)    ? _themeObj.panelBg    : "#1F2937",
        subtext:  (_themeObj && _themeObj.subtext)    ? _themeObj.subtext    : "#9AA0A6",
        accent:   (_themeObj && _themeObj.primary)    ? _themeObj.primary    : "#3B82F6"
    })

    Rectangle { anchors.fill: parent; color: t.bg }

    Pane {
        id: card
        anchors.centerIn: parent
        width: Math.min(sp(520), root.width - 2*t.margin)
        padding: t.margin
        background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
        property int formWidth: Math.min(sp(380), card.width - 2*t.margin)

        Column {
            id: content
            spacing: t.spacing
            anchors.centerIn: parent
            width: card.formWidth

            Label {
                text: "Вход"
                color: t.text
                font.pixelSize: t.fontH1
                font.bold: true
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
            }

            TextField {
                id: tfUser
                placeholderText: "Логин"
                width: parent.width; implicitHeight: t.fieldH
                font.pixelSize: t.fontBody
                leftPadding: sp(12); rightPadding: sp(12)
                background: Rectangle { radius: t.corner; color: t.fieldBg; border.color: t.border }
                Keys.onReturnPressed: tfPass.forceActiveFocus()
                enabled: !loginCtl || !loginCtl.busy
                color: t.text
                placeholderTextColor: t.subtext
            }

            Item {
                width: parent.width
                height: t.fieldH

                TextField {
                    id: tfPass
                    anchors.fill: parent
                    placeholderText: "Пароль"
                    echoMode: TextInput.Password
                    font.pixelSize: t.fontBody
                    leftPadding: sp(12)
                    rightPadding: sp(12 + 36 + 8)
                    background: Rectangle { radius: t.corner; color: t.fieldBg; border.color: t.border }
                    Keys.onReturnPressed: btnLogin.click()   // 🔹 исправлено
                    enabled: !loginCtl || !loginCtl.busy
                    color: t.text
                    placeholderTextColor: t.subtext
                }

                Rectangle {
                    id: eyeBtn
                    width: sp(36); height: sp(36)
                    radius: sp(8)
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: sp(6)
                    color: "transparent"; border.color: "transparent"

                    property bool hovered: false
                    property bool pressed: false
                    property bool open: tfPass.echoMode !== TextInput.Password

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        color: "#000000"
                        opacity: eyeBtn.pressed ? 0.15 : (eyeBtn.hovered ? 0.08 : 0.0)
                        visible: opacity > 0
                    }

                    Canvas {
                        id: icon
                        anchors.centerIn: parent
                        width: sp(20); height: sp(20)
                        onPaint: {
                            const ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = t.subtext;
                            ctx.fillStyle = t.subtext;
                            ctx.lineWidth = Math.max(1, width*0.08);
                            ctx.lineCap = "round";
                            ctx.lineJoin = "round";
                            const w = width, h = height;
                            const cx = w/2, cy = h/2;

                            ctx.beginPath();
                            ctx.moveTo(w*0.1, cy);
                            ctx.quadraticCurveTo(cx, h*0.05, w*0.9, cy);
                            ctx.quadraticCurveTo(cx, h*0.95, w*0.1, cy);
                            ctx.closePath();
                            ctx.stroke();

                            if (eyeBtn.open) {
                                ctx.beginPath();
                                ctx.arc(cx, cy, Math.min(w,h)*0.18, 0, Math.PI*2);
                                ctx.fill();
                            } else {
                                ctx.beginPath();
                                ctx.moveTo(w*0.18, h*0.18);
                                ctx.lineTo(w*0.82, h*0.82);
                                ctx.stroke();
                            }
                        }
                        Component.onCompleted: requestPaint()
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: eyeBtn.hovered = true
                        onExited: eyeBtn.hovered = false
                        onPressed: eyeBtn.pressed = true
                        onReleased: eyeBtn.pressed = false
                        onClicked: {
                            tfPass.echoMode = (tfPass.echoMode === TextInput.Password)
                                              ? TextInput.Normal : TextInput.Password
                            eyeBtn.open = (tfPass.echoMode !== TextInput.Password)
                            icon.requestPaint()
                        }
                    }
                }
            }

            Button {
                id: btnLogin
                width: parent.width; implicitHeight: t.controlH
                contentItem: Label {
                    anchors.fill: parent
                    text: (loginCtl && loginCtl.busy) ? "Входим..." : "Войти"
                    color: t.text; font.pixelSize: t.fontBody
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Item {
                    anchors.fill: parent
                    Rectangle { anchors.fill: parent; radius: t.corner; color: t.panelBg; border.color: t.border }
                    Rectangle { anchors.fill: parent; radius: t.corner; color: "black"
                        opacity: btnLogin.down ? 0.15 : (btnLogin.hovered ? 0.07 : 0.0); visible: opacity>0 }
                }
                MouseArea {
                    anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton
                    cursorShape: (loginCtl && loginCtl.busy) ? Qt.BusyCursor : Qt.PointingHandCursor
                }
                onClicked: {
                    const u = tfUser.text.trim(), p = tfPass.text.trim()
                    if (!u || !p) { err.text = "Введите логин и пароль"; return }
                    if (loginCtl && loginCtl.login) {
                        const ok = loginCtl.login(u, p)
                        console.log("[Login] click user=", u, " pass.len=", p.length, " ->", ok)
                    }
                }
            }

            Label {
                id: err
                color: "#ff8a8a"; width: parent.width
                font.pixelSize: t.fontBody
                wrapMode: Text.WordWrap
                text: (loginCtl && loginCtl.errorText) ? loginCtl.errorText : ""
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Connections {
        target: loginCtl
        enabled: loginCtl !== null
        ignoreUnknownSignals: true
        function onLoginSuccess() {
            if (router && router.goMenu) router.goMenu()
        }
    }
}
