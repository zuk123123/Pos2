import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: page
    property var router
    property string userName: (router && router.currentUserName) ? router.currentUserName : ""

    readonly property var T: (typeof AppTheme !== "undefined" && AppTheme) ? AppTheme : null
    readonly property real s: (T && T.fontScale) ? T.fontScale : 1.0
    function sp(px){ return Math.max(1, Math.round(px*s)) }

    readonly property var t: ({
        fontBody: sp(16), fieldH: sp(44), controlH: sp(48),
        corner: sp(10), spacing: sp(12), margin: sp(16),
        border:   (T && T.border)     ? T.border     : "#333",
        bg:       (T && T.bgColor)    ? T.bgColor    : "#111827",
        panelBg:  (T && T.panelBg)    ? T.panelBg    : "#1F2937",
        text:     (T && T.textColor)  ? T.textColor  : "#F9FAFB",
        subtext:  (T && T.subtext)    ? T.subtext    : "#9AA0A6",
        accent:   (T && T.primary)    ? T.primary    : "#3B82F6"
    })

    Rectangle { anchors.fill: parent; color: t.bg }

    HeaderBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        userName: page.userName
        rightIcon: "🏠"
        rightToolTip: "В меню"
        onRightButtonClicked: if (page.router && page.router.goMenu) page.router.goMenu()
    }

    Flickable {
        id: flick
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentWidth: width
        contentHeight: col.implicitHeight + sp(40)
        clip: true

        ColumnLayout {
            id: col
            width: Math.min(sp(680), flick.width - 2*t.margin)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: t.margin
            spacing: t.spacing

            // Карточка "Интерфейс"
            Frame {
                Layout.fillWidth: true
                background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                padding: t.margin

                ColumnLayout {
                    spacing: t.spacing

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Тема"; color: t.text; font.pixelSize: t.fontBody; Layout.preferredWidth: sp(140) }
                        ComboBox {
                            id: themeBox
                            Layout.fillWidth: true
                            implicitHeight: t.fieldH
                            font.pixelSize: t.fontBody
                            model: (typeof SettingsCtl !== "undefined" && SettingsCtl) ? SettingsCtl.themeNames : ["Dark","Light"]
                            onActivated: { AppTheme.applyByName(currentText); logTheme.text = "Тема → " + currentText }
                            Component.onCompleted: {
                                const cur = (AppTheme && AppTheme.current!==undefined) ? AppTheme.current : "Dark";
                                const i = model.indexOf(cur); currentIndex = i>=0? i:0;
                            }
                            background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                            contentItem: Label { text: themeBox.displayText; color: t.text; verticalAlignment: Text.AlignVCenter; leftPadding: sp(10) }
                        }
                        Button {
                            id: saveBtn
                            text: "Сохранить"
                            implicitHeight: t.controlH
                            onClicked: {
                                if (typeof SettingsCtl !== "undefined" && SettingsCtl)
                                    SettingsCtl.saveUserTheme(themeBox.currentText)
                            }
                            contentItem: Label { anchors.centerIn: parent; text: parent.text; color: t.text; font.pixelSize: t.fontBody }
                            background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                        }
                    }
                    Label { id: logTheme; text: ""; color: t.subtext; font.pixelSize: sp(12) }
                }
            }

            // Конструктор темы
            Frame {
                Layout.fillWidth: true
                background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                padding: t.margin

                ColumnLayout {
                    spacing: t.spacing
                    Label { text: "Конструктор темы"; color: t.text; font.pixelSize: t.fontBody; font.bold: true }

                    GridLayout {
                        columns: 2; rowSpacing: t.spacing; columnSpacing: t.spacing
                        function field(lbl, tf) {
                            return RowLayout { Layout.fillWidth: true; spacing: t.spacing; Label { text: lbl; color: t.text; font.pixelSize: t.fontBody; Layout.preferredWidth: sp(170) }; tf }
                        }

                        // Имя
                        RowLayout {
                            Layout.columnSpan: 2
                            Label { text: "Имя темы"; color: t.text; font.pixelSize: t.fontBody; Layout.preferredWidth: sp(170) }
                            TextField {
                                id: nameTf; Layout.fillWidth: true; implicitHeight: t.fieldH; font.pixelSize: t.fontBody
                                placeholderText: "MyTheme"; text: ""
                                background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                                color: t.text; placeholderTextColor: t.subtext
                            }
                        }

                        // Цвета (hex)
                        Repeater {
                            model: [
                                {k:"primary",    lbl:"Основной цвет",   def:"#3B82F6"},
                                {k:"background", lbl:"Фон окна",        def:"#111827"},
                                {k:"panel",      lbl:"Фон панелей",      def:"#1F2937"},
                                {k:"text",       lbl:"Текст",            def:"#F9FAFB"},
                                {k:"border",     lbl:"Границы",          def:"#333333"}
                            ]
                            delegate: RowLayout {
                                Label { text: modelData.lbl; color: t.text; font.pixelSize: t.fontBody; Layout.preferredWidth: sp(170) }
                                TextField {
                                    id: tf
                                    property string key: modelData.k
                                    text: modelData.def
                                    implicitHeight: t.fieldH; Layout.fillWidth: true; font.pixelSize: t.fontBody
                                    background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                                    color: t.text; placeholderTextColor: t.subtext
                                }
                            }
                        }
                    }

                    // Превью
                    Rectangle {
                        Layout.fillWidth: true; height: sp(80); radius: t.corner
                        color: panelTf.text || "#1F2937"
                        border.color: borderTf.text || "#333333"
                        Row {
                            anchors.centerIn: parent; spacing: sp(12)
                            Rectangle { width: sp(28); height: sp(28); radius: sp(6); color: primaryTf.text || "#3B82F6" }
                            Label { text: "Текст"; color: textTf.text || "#F9FAFB"; font.pixelSize: t.fontBody }
                        }
                    }

                    // Кнопки
                    RowLayout {
                        spacing: t.spacing
                        Button {
                            text: "Сохранить тему"
                            implicitHeight: t.controlH
                            onClicked: {
                                if (!nameTf.text.trim()) return;
                                if (typeof SettingsCtl !== "undefined" && SettingsCtl) {
                                    SettingsCtl.createOrUpdateTheme(
                                        nameTf.text.trim(),
                                        primaryTf.text.trim(),
                                        backgroundTf.text.trim(),
                                        textTf.text.trim(),
                                        borderTf.text.trim(),
                                        panelTf.text.trim()
                                    )
                                }
                            }
                            contentItem: Label { anchors.centerIn: parent; text: parent.text; color: t.text; font.pixelSize: t.fontBody }
                            background: Rectangle { radius: t.corner; color: t.panelBg; border.color: t.border }
                        }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            Item { Layout.fillHeight: true }
        }
    }
}
