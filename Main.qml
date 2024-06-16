import QtQuick

import WebcamCaptureQml 1.0
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("LivenessChecker")
    minimumWidth: 1280
    minimumHeight: 720

    onClosing: {
        webcamCapture.stop()
    }

    WebcamCapture {
        id: webcamCapture
        onImgReady: function(some_img) {
            ImageProvider.acceptImage(some_img)
            rgb_img.curr_image = Math.random().toString()
        }
        onSendMessage: function (json_message){
            var JsonObject= JSON.parse(json_message);
            messages_model.insert(0, JsonObject)
        }
        onWorker_thread_stoped: {
            rgb_img.curr_image = "default_image"
            messages_model.insert(0, {"message_from_cpp": "Окончание сессии", "color_from_cpp": "#80c342"})
        }
        onWorker_thread_started: {
            messages_model.insert(0, {"message_from_cpp": "Старт сессии", "color_from_cpp": "#80c342"})
        }
        onSet_bestshot: function(base64Image)  {
            last_face_image.source = base64Image
        }
        onSessionAlreadyRunning: {
            messages_model.insert(0, {"message_from_cpp": "Сессия уже запущена", "color_from_cpp": "white"})
        }
    }

    Row {
        id: buttons_row
        anchors {
            horizontalCenter: parent.horizontalCenter
        }
        topPadding: 30
        spacing: 2
        property int btns_w: 80
        property int btns_h: 20
        height: buttons_row.btns_h
        width: buttons_row.btns_w * 2 + spacing

        Button {
            id: start_btn
            width: buttons_row.btns_w
            height: buttons_row.btns_h
            onClicked: {
                ImageProvider.startVideoRunning()
                webcamCapture.start()
            }
            text: "Начать"
        }
        Button {
            id: stop_btn
            width: buttons_row.btns_w
            height: buttons_row.btns_h
            onClicked: {
                ImageProvider.stopVideoRunning()
                webcamCapture.stop()
                last_face_image.source = last_face_image.default_bs_image
            }
            text: "Остановить"
        }
    }

    Row {
        id: screens
        property int left_right_margin: 90
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: screens.left_right_margin
            rightMargin: screens.left_right_margin
            top: buttons_row.bottom
            topMargin: 50
        }
        spacing: 20
        height: (root.height - buttons_row.height - buttons_row.topPadding - screens.anchors.topMargin) * 0.6
        Rectangle {
            id: messages_rect
            border.color: "cyan"
            border.width: 1
            width: (root.width - screens.left_right_margin * 2 - screens.spacing * 2) * 0.2
            height: screens.height
            ListModel {
                id: messages_model
            }
            Component {
                id: messages_model_delegate
                Rectangle {
                    width: messages_view.width
                    height: 30
                    color: color_from_cpp
                    Text {
                        anchors.fill: parent
                        text: message_from_cpp
                        color: "black"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }
            ListView {
                id: messages_view
                anchors.fill: parent
                anchors.margins: 1
                model: messages_model
                delegate: messages_model_delegate
                clip: true
            }
        }
        Rectangle {
            id: rgb_rect
            border.color: "cyan"
            border.width: 1
            width: (root.width - screens.left_right_margin * 2 - screens.spacing * 2) * 0.6
            height: screens.height
            Image {
                id: rgb_img
                anchors.centerIn: parent
                width: curr_image === "default_image" ? rgb_rect.width * 0.35 : rgb_rect.width - rgb_rect.border.width * 2
                height: curr_image === "default_image" ? rgb_rect.height * 0.35 : rgb_rect.height - rgb_rect.border.width * 2
                fillMode: curr_image === "default_image" ? Image.PreserveAspectFit : Image.Stretch
                cache: false
                property string curr_image: "default_image"
                source: "image://ImageProvider/" + curr_image
            }
            Text {
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 5
                    rightMargin: 5
                }
                font.pixelSize: 17
                text: "RGB"
                color: "cyan"
            }
        }
        Column {
            id: col
            width: (root.width - screens.left_right_margin * 2 - screens.spacing * 2) * 0.2
            spacing: 30
            height: screens.height
            Rectangle {
                id: ir_rect
                border.color: "cyan"
                border.width: 1
                width: col.width
                height: (col.height - col.spacing) / 2
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/qml_images/video.png"
                    width: ir_rect.width * 0.35
                    height: ir_rect.height * 0.35
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                    anchors {
                        top: parent.top
                        right: parent.right
                        topMargin: 5
                        rightMargin: 5
                    }
                    font.pixelSize: 17
                    text: "IR"
                    color: "cyan"
                }
            }
            Rectangle {
                id: depth_rect
                border.color: "cyan"
                border.width: 1
                width: col.width
                height: (col.height - col.spacing) / 2
                Image {
                    anchors.centerIn: parent
                    source: "qrc:/qml_images/video.png"
                    width: ir_rect.width * 0.35
                    height: ir_rect.height * 0.35
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                    anchors {
                        top: parent.top
                        right: parent.right
                        topMargin: 5
                        rightMargin: 5
                    }
                    font.pixelSize: 17
                    text: "Depth"
                    color: "cyan"
                }
            }
        }
    }
    Column {
        id: last_alive_face
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: screens.bottom
            bottom: parent.bottom
        }
        topPadding: 30
        bottomPadding: 20
        width: parent.width * 0.2
        spacing: 10
        Text {
            id: last_face_text
            text: "Последнее обнаруженное лицо"
            height: 30
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 14
        }
        Rectangle {
            id: last_face_rect
            border.width: 1
            border.color: "cyan"
            width: parent.width
            color: "transparent"
            height: parent.height - parent.topPadding - parent.bottomPadding - last_face_text.height
            Image {
                id: last_face_image
                anchors.centerIn: last_face_rect
                property string default_bs_image: "qrc:/qml_images/face_recognition.png"
                source: default_bs_image
                fillMode: Image.PreserveAspectFit
                width: last_face_rect.width - last_face_rect.border.width * 2
                height: last_face_rect.height - last_face_rect.border.width * 2
            }
        }
    }
}
