import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: page
    property var router
    property string userName: (router && router.currentUserName) ? router.currentUserName : ""

    readonly property var _themeObj: (typeof AppTheme !== "undefined" && AppTheme) ? AppTheme : null
    readonly property real s: (_themeObj && _themeObj.fontScale) ? _themeObj.fontScale : 1.0
    function sp(px){ return Math.max(1, Math.round(px*s)) }

    readonly property var t: ({
        fontBody: sp(16),
        controlH: sp(52), corner: sp(10),
        spacing: sp(14), margin: sp(16),
        border:   (_themeObj && _themeObj.border)     ? _themeObj.border     : "#333",
        bg:       (_themeObj && _themeObj.bgColor)    ? _themeObj.bgColor    : "#111827",
        panelBg:  (_themeObj && _themeObj.panelBg)    ? _themeObj.panelBg    : "#1F2937",
        text:     (_themeObj && _themeObj.textColor)  ? _themeObj.textColor  : "#F9FAFB",
        subtext:  (_themeObj && _themeObj.subtext)    ? _themeObj.subtext    : "#9AA0A6",
        accent:   (_themeObj && _themeObj.primary)    ? _themeObj.primary    : "#3B82F6"
    })

    Rectangle { anchors.fill: parent; color: t.bg }

    HeaderBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        userName: page.userName
        rightIcon: "🔒"
        rightToolTip: "Выйти"
        onRightButtonClicked: {
            if (!page.router) return
            if (page.router.setUser) page.router.setUser("")
            if (page.router.goLogin) page.router.goLogin()
        }
    }

    Item {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom

        Column {
            id: col
            spacing: t.spacing
            width: Math.min(sp(520), content.width - 2*t.margin)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: t.margin

            Component {
                id: menuButton
                Button {
                    id: btn
                    width: col.width
                    implicitHeight: t.controlH
                    property string label: ""
                    property string iconText: ""
                    property var onGo: null

                    contentItem: Item {
                        anchors.fill: parent
                        anchors.margins: sp(12)

                        // иконка слева
                        Label {
                            id: leftIcon
                            text: iconText
                            color: t.text
                            font.pixelSize: t.fontBody
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            width: implicitWidth
                        }
                        // текст строго по центру кнопки
                        Label {
                            text: label
                            color: t.text
                            font.pixelSize: t.fontBody
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            // ширина без учёта области слева под иконку
                            width: parent.width - Math.max(leftIcon.width, sp(20)) - sp(20)
                        }
                    }

                    background: Item {
                        anchors.fill: parent
                        Rectangle { anchors.fill: parent; radius: t.corner; color: t.panelBg; border.color: t.border }
                        Rectangle { anchors.fill: parent; radius: t.corner; color: "black"
                            opacity: btn.down ? 0.15 : (btn.hovered ? 0.07 : 0.0)
                            visible: opacity > 0
                        }
                    }

                    onClicked: { if (typeof onGo === "function") onGo() }
                    Keys.onReturnPressed: clicked()

                    MouseArea { anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.NoButton; cursorShape: Qt.PointingHandCursor }
                }
            }

            Loader { sourceComponent: menuButton; onLoaded: { item.label="Касса";             item.iconText="🛒"; item.onGo=function(){ if (page.router&&page.router.goCashier)   page.router.goCashier() } } }
            Loader { sourceComponent: menuButton; onLoaded: { item.label="Склад";             item.iconText="📦"; item.onGo=function(){ if (page.router&&page.router.goWarehouse) page.router.goWarehouse() } } }
            Loader { sourceComponent: menuButton; onLoaded: { item.label="Инвентаризация";    item.iconText="🧾"; item.onGo=function(){ if (page.router&&page.router.goInventory)  page.router.goInventory() } } }
            Loader { sourceComponent: menuButton; onLoaded: { item.label="Аналитика";         item.iconText="📈"; item.onGo=function(){ if (page.router&&page.router.goAnalytics)  page.router.goAnalytics() } } }
            Loader { sourceComponent: menuButton; onLoaded: { item.label="Настройки";         item.iconText="⚙️"; item.onGo=function(){ if (page.router&&page.router.goSettings)   page.router.goSettings() } } }
            Loader { sourceComponent: menuButton; onLoaded: { item.label="Администрирование"; item.iconText="🛠"; item.onGo=function(){ if (page.router&&page.router.goAdmin)      page.router.goAdmin() } } }

            Item { width: 1; height: t.margin }
        }
    }
}
