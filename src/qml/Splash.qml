import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property var router

    Rectangle { anchors.fill: parent; color: Theme.background }

    BusyIndicator {
        anchors.centerIn: parent
        running: true
        width: Math.max(48, 56 * Theme.uiScale)
        height: width
    }

    property bool minDelayPassed: false
    property bool bootDone: false

    Timer {
        interval: 800
        running: true
        repeat: false
        onTriggered: { minDelayPassed = true; maybeGo() }
    }

    Component.onCompleted: {
        if (typeof Boot !== "undefined" && Boot.start) {
            console.log("[Splash] Boot.start()");
            Boot.start();
        } else {
            console.warn("[Splash] Boot bridge missing, skipping ping");
            bootDone = true;
        }
        checkT.start();
    }

    Connections {
        target: Boot
        enabled: typeof Boot !== "undefined"
        function onFinished(ok) {
            console.log("[Splash] Boot finished. ok =", ok);
            bootDone = true; maybeGo();
        }
    }

    function maybeGo() {
        if (minDelayPassed && (bootDone || typeof Boot === "undefined")) {
            console.log("[Splash] goLogin()");
            if (root.router && root.router.goLogin) root.router.goLogin();
            checkT.stop();
        }
    }

    Timer { id: checkT; interval: 80; running: false; repeat: true; onTriggered: maybeGo() }
}
