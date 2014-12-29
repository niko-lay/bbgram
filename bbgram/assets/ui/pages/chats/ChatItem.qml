import bb.cascades 1.2

CustomListItem {
    property bool selected : false
    preferredHeight: 100
    
    
    onSelectedChanged: {
        if (selected) {
            background.background = Color.create("#31A3DD")
            opacity = 0.2
        }
        else {
            background.background = Color.White
            background.opacity = 1
        }
    }
    
    content: Container {
        id:background
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        
        ImageView {
            verticalAlignment: VerticalAlignment.Center
            
            //image: ListItemData.photo
            imageSource: ListItemData.photo
            scalingMethod: ScalingMethod.AspectFit
            preferredHeight: 100
            preferredWidth: 100
            minHeight: 100
            minWidth: 100
        }
        
        Container {
            layout: StackLayout {
            }
            verticalAlignment: VerticalAlignment.Center
            leftPadding: 10
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                rightPadding: 20
                Container {
                    layoutProperties: StackLayoutProperties  {
                        spaceQuota: 1
                    }
                    /*ImageView {
                         imageSource: "asset:///images/secretchat_green.png"
                         preferredWidth: 40
                         preferredHeight: 40
                         verticalAlignment: VerticalAlignment.Center
                     }*/
                    Label {
                        text: ListItemData.title
                        //text: "firstName lastName"
                        verticalAlignment: VerticalAlignment.Center
                        textStyle {
                            fontSize: FontSize.Large
                        }
                    }
                }
                Container { 
                    leftPadding: 40
                    Label {
                        text: ListItemData.lastMessage ? ListItemData.lastMessage.dateFormatted : ""
                        //text: "Feb 21"
                        verticalAlignment: VerticalAlignment.Center
                        textStyle {
                            color: Color.Gray
                        }
                    }               
                }
            }
            
            Container {
                layout: StackLayout {
                    orientation: LayoutOrientation.LeftToRight
                }
                rightPadding: 20
                Container {
                    layoutProperties: StackLayoutProperties  {
                        spaceQuota: 1
                    }
                    Label {
                        text: ListItemData.status
                        //text: "As they rounded a bend in the path that ran beside the river, Lara recognized the silhouette of a fig tree atop a nearby hill."
                        textFormat: TextFormat.Html
                        textStyle {
                            color: Color.Gray
                            fontSize: FontSize.Medium
                        }                    
                        horizontalAlignment: HorizontalAlignment.Left
                        verticalAlignment: VerticalAlignment.Center
                    }
                }
                ControlDelegate {
                    sourceComponent: unreadCountCompDef
                    delegateActive: false
                }
            }
        }
    
    }
    attachedObjects: [
        ComponentDefinition {
            id: unreadCountCompDef
            Container {
                layoutProperties: StackLayoutProperties  {
                    spaceQuota:-1 
                }
                leftPadding: 40
                
                verticalAlignment: VerticalAlignment.Center
                
                Container {
                    layout: DockLayout {}
                    background: Color.create("#74B266")
                    minWidth: 45
                    leftPadding: 4
                    rightPadding: 4
                    bottomPadding: 4
                    horizontalAlignment: HorizontalAlignment.Right
                    verticalAlignment: VerticalAlignment.Bottom
                    
                    Label {
                        text: "3"
                        horizontalAlignment: HorizontalAlignment.Center
                        verticalAlignment: VerticalAlignment.Center
                        textStyle {
                            color: Color.White
                            textAlign: TextAlign.Center
                            fontSize: FontSize.Small
                            fontWeight: FontWeight.Bold
                        }
                    
                    }
                }
            }
        }
    ]
}

