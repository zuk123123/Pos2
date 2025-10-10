import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: win
    visible: true
    width: Theme.screenWidth
    height: Theme.screenHeight
    title: "pos2"
    background: Rectangle { color: Theme.background }

    // простой роутер
    QtObject {
        id: router
        function goSplash() { page.sourceComponent = splash }
        function goLogin()  { page.sourceComponent = login  }
        function goMenu()   { page.sourceComponent = home   }
    }

    Loader {
        id: page
        anchors.fill: parent
        sourceComponent: splash
        onLoaded: {
            if (item && item.hasOwnProperty && item.hasOwnProperty("router"))
                item.router = router;
        }
    }

    Component { id: splash; Splash { } }
    Component { id: login;  Login  { } }

    // простая заглушка "Home"
    Component {
        id: home
        Item {
            anchors.fill: parent
            Rectangle { anchors.fill: parent; color: Theme.background }
            Label {
                anchors.centerIn: parent
                text: "HOME (заглушка)"
                color: Theme.text
                font.pixelSize: Theme.fontLarge
            }
        }
    }
}
