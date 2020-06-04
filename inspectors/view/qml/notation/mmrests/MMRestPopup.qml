import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

StyledPopup {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Column {
            spacing: 8

            height: childrenRect.height
            width: parent.width

            StyledTextLabel {
                anchors.left: parent.left
                text: qsTr("Number position")
            }

            Item {
                height: childrenRect.height
                width: parent.width

                IncrementalPropertyControl {
                    id: numberPositionControl

                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    icon: IconCode.VERTICAL
                    isIndeterminate: root.model ? root.model.numberPosition.isUndefined : false
                    currentValue: root.model ? root.model.numberPosition.value : 0

                    measureUnitsSymbol: "sp"
                    step: 0.5
                    decimals: 2
                    maxValue: 99.00
                    minValue: -99.00

                    onValueEdited: { root.model.numberPosition.value = newValue }
                }
            }
        }
    }
}
