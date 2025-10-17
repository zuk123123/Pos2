import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: bar
    height: 56
    color: (typeof AppTheme !== "undefined" && AppTheme && AppTheme.panelBg !== undefined)
           ? AppTheme.panelBg : "#1F2937"
    border.color: (typeof AppTheme !== "undefined" && AppTheme && AppTheme.border !== undefined)
           ? AppTheme.border : "#333333"

    // слева – имя пользователя
    property string userName: ""

    // справа – иконка (🔒 в меню, 🏠 на других страницах)
    property string rightIcon: "🏠"
    property string rightToolTip: ""
    signal rightButtonClicked()

    readonly property var _themeObj: (typeof AppTheme !== "undefined" && AppTheme) ? AppTheme : null
    readonly property real s: (_themeObj && _themeObj.fontScale) ? _themeObj.fontScale : 1.0
    function sp(px){ return Math.max(1, Math.round(px*s)) }

    Label {
        id: leftText
        text: userName ? userName : ""
        color: (AppTheme && AppTheme.textColor) ? AppTheme.textColor : "#F9FAFB"
        font.pixelSize: sp(16)
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: sp(12)
        elide: Text.ElideRight
        width: parent.width * 0.6
    }

    ToolButton {
        id: rightBtn
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: sp(8)
        implicitHeight: sp(36)
        implicitWidth: sp(40)

        contentItem: Label {
            anchors.fill: parent
            text: bar.rightIcon
            color: (AppTheme && AppTheme.textColor) ? AppTheme.textColor : "#F9FAFB"
            font.pixelSize: sp(18)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        background: Item {
            anchors.fill: parent
            Rectangle { anchors.fill: parent; radius: sp(8);
                color: (AppTheme && AppTheme.panelBg) ? AppTheme.panelBg : "#1F2937";
                border.color: (AppTheme && AppTheme.border)  ? AppTheme.border  : "#333333"
            }
            Rectangle { anchors.fill: parent; radius: sp(8); color: "black"
                opacity: rightBtn.down ? 0.15 : (rightBtn.hovered ? 0.07 : 0.0)
                visible: opacity > 0
            }
        }
        hoverEnabled: true
        ToolTip.visible: hovered && !!bar.rightToolTip
        ToolTip.text: bar.rightToolTip

        onClicked: bar.rightButtonClicked()

        MouseArea { anchors.fill: parent; acceptedButtons: Qt.NoButton; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
    }
}
