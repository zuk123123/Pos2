import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: win
    width: 900
    height: 600
    visible: true
    title: "Pos2"

    property string currentPage: ""
    property string currentUserName: ""
    property string bootStage: "Загрузка ресурсов…"

    background: Rectangle {
        color: (typeof AppTheme !== "undefined" && AppTheme && AppTheme.bgColor !== undefined)
               ? AppTheme.bgColor : "#121212"
    }

    QtObject {
        id: router
        function setUser(name) { win.currentUserName = name || "" }
        function goLogin()     { go("login") }
        function goMenu()      { go("menu") }
        function goSettings()  { go("settings") }
        function go(page) {
            if (win.currentPage === page) return;
            win.currentPage = page;
            switch (page) {
            case "login":    view.source = "qrc:/Login.qml";     break;
            case "menu":     view.source = "qrc:/Menu.qml";      break;
            case "settings": view.source = "qrc:/Settings.qml";  break;
            default:         view.source = "";                   break;
            }
        }
        readonly property string currentUserName: win.currentUserName
    }

    Loader {
        id: view
        anchors.fill: parent
        onLoaded: {
            if (item && item.hasOwnProperty && item.hasOwnProperty("router"))
                item.router = router;
            if (item && item.hasOwnProperty && item.hasOwnProperty("userName"))
                item.userName = win.currentUserName;
        }
    }

    // SPLASH overlay
    Item {
        anchors.fill: parent
        visible: (win.currentPage === "")

        Rectangle { anchors.fill: parent; color: (typeof AppTheme!=="undefined"&&AppTheme)?AppTheme.bgColor:"#111827" }

        Column {
            id: splashCol
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(parent.width * 0.8, 480)

            // кружок строго по центру
            BusyIndicator {
                running: true
                width: 56; height: 56
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // текст по центру, меньше шрифт
            Label {
                text: win.bootStage
                color: (typeof AppTheme!=="undefined"&&AppTheme)?AppTheme.textColor:"#ffffff"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    // этапы
    Timer {
        interval: 150
        running: true
        repeat: false
        onTriggered: win.bootStage = "Инициализация БД…"
    }

    Connections {
        target: Boot
        ignoreUnknownSignals: true
        function onFinished(ok) {
            win.bootStage = ok ? "Проверка соединения… OK" : "Проверка соединения… нет связи";
            router.goLogin();
        }
    }

    Connections {
        target: Api
        ignoreUnknownSignals: true
        function onPingOk() {
            if (win.currentPage === "") win.bootStage = "Проверка соединения… OK";
        }
    }

    Component.onCompleted: {
        currentPage = "";
        view.source = "";
        bootStage = "Загрузка ресурсов…";
    }
}
