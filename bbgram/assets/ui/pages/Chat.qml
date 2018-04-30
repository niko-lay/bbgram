import bb.cascades 1.2
import bb.system 1.0
import bbgram.types.lib 0.1
import bb.cascades.pickers 1.0

import "chats"

Page {
    objectName: "Chat"
    id: me
    property Peer peer: null
    
    function messageAdded(indexPath) {
        if (indexPath.length != 0 && indexPath[0] == 0)
            messageList.scrollToPosition(ScrollPosition.Beginning, ScrollAnimation.None);
        if (!messageList.dataModel.data(indexPath).our)
            _owner.markRead(peer);
    }
    
    function onPop() {
        messageList.dataModel.itemAdded.disconnect(messageAdded)
        
        input.recorder.startRecord.disconnect(startRecord)
        input.recorder.stopRecord.disconnect(stopRecord)
        input.recorder.cancelRecord.disconnect(cancelRecord)
    }
    
    function onPush() {
        _owner.markRead(peer);
        
        messageList.dataModel.itemAdded.connect(messageAdded)
        
        input.recorder.startRecord.connect(startRecord)
        input.recorder.stopRecord.connect(stopRecord)
        input.recorder.cancelRecord.connect(cancelRecord)
    }
    
    function sendMessage() {
       _owner.sendMessage(peer, input.value)
       input.clear()
    }
    
    function startRecord(){
        _owner.startRecord();
    }
    
    function stopRecord(){
        if (input.recorder.duration > 2)
            _owner.sendAudio(peer, _owner.stopRecord(), input.recorder.duration);
        else
            _owner.stopRecord();
    }
    
    function cancelRecord(){
        _owner.stopRecord()
    }
    
    function clearHistory() {
        _owner.deleteHistory(peer)
    }
    
    function about() {
        if (peer.type == 1) {// user
            var page = contactPageDef.createObject()
            page.user = peer
            var navigationPane = Application.scene.activeTab.content 
            navigationPane.push(page)
        }
        else if (peer.type == 2 || peer.type == 255) { // group or broadcast
            var page = groupPageDef.createObject()
            page.chat = peer
            var navigationPane = Application.scene.activeTab.content 
            navigationPane.push(page)
        }
    }
    
    function addParticipant(user, sheet)
    {
        sheet.userSelected.disconnect(addParticipant)
        
        if (user)
            _owner.addUserToGroup(peer, user)
    }
    
    titleBar: TitleBar {
        kind: TitleBarKind.FreeForm
        scrollBehavior: TitleBarScrollBehavior.Sticky
        kindProperties: FreeFormTitleBarKindProperties {
            Container {
                gestureHandlers: [
                    TapHandler {
                        onTapped: {
                            about()
                        }
                    }
                ]
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                
                ImageView {
                    id: avatar
                    imageSource: peer ? peer.photo : ""
                    //imageSource: "asset:///images/placeholders/user_placeholder_purple.png"
                    scalingMethod: ScalingMethod.AspectFit
                }
                
                attachedObjects: [
                    LayoutUpdateHandler {
                        onLayoutFrameChanged: {
                            avatar.maxWidth = layoutFrame.height
                            avatar.maxHeight = layoutFrame.height
                        }
                    }
                ]
                
                Container {
                    layout: StackLayout {
                    }
                    verticalAlignment: VerticalAlignment.Center
                    leftPadding: 20
                    Label {
                        text: peer ? peer.title : ""
                        //text: "Anastasiya Shy"
                        textStyle.base: titleTextStyle.style
                        bottomMargin: 0
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    Label {
                        text: 
                        if (peer) {
                            if (peer.type == 1)
                                peer.online ? "online" : "last seen " + peer.lastSeenFormatted
                            else if (peer.type == 2) {
                                var n = peer.members.size();
                                var o = 0;
                                for (var i = 0; i < n; i++) {
                                    var user = peer.members.data([i])
                                    if (user.online)
                                        o++
                                }
                                "%1 members, %2 online".arg(n).arg(o) 
                            }
                        
                        }
                        else  
                            ""
                        topMargin: 0
                        textStyle.base: titleStatusTextStyle.style
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                }
            }
        }
    }
    
    actions: [
        ActionItem {
            id: attachAction
            title: "Attach"
            imageSource: "asset:///images/bar_attach.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            
            onTriggered: {
                filePicker.open()
            }
            
            attachedObjects: [
                FilePicker {
                    id: filePicker
                    type : FileType.Picture
                    title : "Select Picture"
                    mode: FilePickerMode.Picker
                    directories : ["/accounts/1000/shared/"]
                    onFileSelected : {
                        _owner.sendPhoto(peer, selectedFiles[0]);
                    }
                }
            ]
        },
        ActionItem {
            id: sendAction
            enabled: false
            title: "Send"
            imageSource: "asset:///images/bar_send.png"
            ActionBar.placement: ActionBarPlacement.OnBar
            
            onTriggered: {
                me.sendMessage()
            }
        },
        ActionItem {
            id: callAction
            title: "Call"
            imageSource: "asset:///images/menu_phone.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                if (peer.phone.length === 0){
                    toastNoPhone.show()
                }
                else{
                    _owner.dialANumber("+" + peer.phone)
                }
            }
        },
        ActionItem {
            id: aboutAction
            title: "About"
            imageSource: "asset:///images/bar_profile.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                about();
            }
        },
        ActionItem {
            id: addParticipantAction
            title: "Add Participant"
            imageSource: "asset:///images/menu_bar_contact_plus.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                var sheet = contactPickerSheetDef.createObject();
                sheet.caption = "Add Participant"
                
                sheet.userSelected.connect(addParticipant);
                
                sheet.open()
            }
        },
        ActionItem {
            id: sharedMediaAction
            title: "Shared Media"
            imageSource: "asset:///images/menu_sharedmedia.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: {
                var page = sharedMediaPageDef.createObject()
                page.peer = peer
                var navigationPane = Application.scene.activeTab.content 
                navigationPane.push(page)
            }
        },
        DeleteActionItem {
            id: clearHistoryAction
            title: "Clear Chat"
            imageSource: "asset:///images/menu_bin.png"
            ActionBar.placement: ActionBarPlacement.InOverflow
            onTriggered: confirmDialog.show()
            attachedObjects: [
                SystemDialog {
                    id: confirmDialog
                    title: clearHistoryAction.title
                    body: "Are you sure you want to clear history?"
                    onFinished: {
                        if (value == SystemUiResult.ConfirmButtonSelection)
                            clearHistory()
                    }
                }
            ]
        }
    ]
    
    onPeerChanged: {
        if (!peer)
            return
        if (peer.type != 1)    // User
            removeAction(callAction)
        if (peer.type != 2)  // Group
            removeAction(addParticipantAction)
        if (peer.type == 2) {
            clearHistoryAction.title = "Clear Conversation"
            aboutAction.imageSource = "asset:///images/menu_group.png"
        }
    }
    
    
    Container {
        layout: StackLayout {
        }
        Container {
            layout: DockLayout {            
            }
            Wallpaper {
            }
            MessageList {
                id: messageList
                peer: me.peer
            }
        }
        MessageInput {
            id: input
            
            onValueChanging: {
                sendAction.enabled = value.length > 0;
            }
            
            onSubmitted: {
                sendMessage();
            }
        }
    }
    
    attachedObjects: [
        SystemToast {
            id: toastNoPhone
            body: "Phone number not avaliable"
            onFinished: {}
        },
        TitleTextStyleDefinition {
            id: titleTextStyle
            fontSize: FontSize.Large
        },
        TitleTextStyleDefinition {
            id: titleStatusTextStyle
            fontSize: FontSize.Small
        },
        ComponentDefinition {
             id: contactPageDef
             source: "ContactInfo.qml"
        },
        ComponentDefinition {
            id: groupPageDef
            source: "GroupInfo.qml"
        },
        ComponentDefinition {
            id: sharedMediaPageDef
            source: "SharedMedia.qml"
        },
        ComponentDefinition {
            id: contactPickerSheetDef
            source: "contacts/ContactPicker.qml"
        }
    ]
}