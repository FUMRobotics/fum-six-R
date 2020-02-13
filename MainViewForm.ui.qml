import QtQuick 2.12
import QtQuick.Controls 2.12
import MainViewModel 1.0


Item {
    Resource
    {
        id:rsc
    }

    //    MFrame{
    //        anchors.fill: parent


    //    }
    Component.onCompleted: {
        mainview1.IsPowerOn ? poweron._isActive = true:poweron._isActive = false
    }
    Grid{
        anchors.fill: parent
        rows: 2
        spacing: 5
        leftPadding: 5
        topPadding: 5
        Grid{
            //anchors.fill: parent
            height: parent.height*0.15
            width: parent.width - 10
            columns: 3
            spacing: 5
            MButton {
                id : poweron
                _text: "Power On"
                _width:parent.width * 0.33
                onBtnClick: {
                    mainview1.PowerOnClicked()
                    mainview1.IsPowerOn ? poweron._isActive = true:poweron._isActive = false
                    //poweron._isActive = true
                }
            }
            MButton {
                id : run
                _text: "Run"
                _width:parent.width * 0.33
                onBtnClick: {
                    mainview1.RunClicked()
                    //run._isActive = true
                }
            }
            MButton {
                id : home
                _text: "Home Position"
                _width:parent.width * 0.33
                onBtnClick: {
                    mainview1.HomePositionClicked()
                    //home._isActive = true
                }
            }
        }
        MFrame
        {
            width: parent.width - 9
            height: parent.height*0.85 - 10
            CodeWindow{

            }

            //            Label{
            //                width: parent.width
            //                height: parent.height
            //                horizontalAlignment: Text.AlignHCenter
            //                verticalAlignment: Text.AlignVCenter
            //                text: "<b>" + "CODES" + "</b>"
            //                color: "#21be2b"
            //            }
        }
    }
    MainViewModel {
        id: mainview1
    }

}

