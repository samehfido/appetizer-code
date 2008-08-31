{*------------------------------------------------------------------------------
  This class handle the panel that contains the icons.

  @Author Laurent Cozic
-------------------------------------------------------------------------------}


unit IconPanel;

interface

uses Windows, WNineSlicesPanel, Classes, WFileIcon, WImage, Logger, Contnrs, Controls,
	Types, WComponent, ExtCtrls, Forms, SysUtils, Graphics, MathUtils, Imaging,
  ShellAPI, WImageButton, Menus, User, StringUtils, EditFolderItemUnit, SystemUtils,
  IconToolTipUnit;


type


  TIconDragData = record
  	Icon: TWComponent;
  	Timer: TTimer;
    MouseDownLoc: TPoint;
    Started: Boolean;
    StartIconLoc: TPoint;
    IconForm: TForm;
  end;


	TIconPanel = class(TWNineSlicesPanel)


  	private

    	pIcons: TObjectList;
      pIconSize: Word;
      pIconDragData: TIconDragData;
      pAutoSize: Boolean;
      pBrowseButton: TWImageButton;
      pLastVisibleIconIndex: Integer;
      pTooltipForm: TIconTooltipForm;

      function CreateFormPopupMenu():TPopupMenu;
      function GetInsertionIndexAtPoint(const aPoint: TPoint; replacementBehavior: Boolean):Integer;
      procedure iconDragData_timer(Sender: TObject);
      procedure icon_mouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
      procedure icon_mouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
      procedure icon_click(sender: TObject);
      procedure Icon_MouseEnter(sender: TObject);
      procedure Icon_MouseExit(sender: TObject);
      procedure iconForm_paint(Sender: TObject);
      procedure Self_MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
      procedure UpdateFolderItemsOrder();
      procedure BrowseButton_Click(Sender: TObject);
      function GetComponentByID(const componentID: Integer): TWComponent;

      procedure BrowseButtonPopupMenu_Click(Sender: TObject);
      
      procedure MenuItem_Click_NewShortcut(Sender: TObject);
      procedure MenuItem_Click_NewSeparator(Sender: TObject);
      procedure MenuItem_Click_Delete(Sender: TObject);
			procedure MenuItem_Click_Properties(Sender: TObject);
      procedure MenuItem_Click_AddItToQuickLaunch(Sender: TObject);

      function FolderItemToComponent(const folderItem:TFolderItem): TWComponent;

      procedure SetAutoSize(const value:Boolean);
      function GetPotentialWidth():Integer;

  	public

    	constructor Create(AOwner: TComponent); override;
      procedure LoadFolderItems();
      function IconCount():Word;
      procedure UpdateLayout();
      property AutoSize:Boolean read pAutoSize write SetAutoSize;
      property PotentialWidth:Integer read GetPotentialWidth;


  end;


implementation


uses Main;



constructor TIconPanel.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);

  Width := 200;

  pLastVisibleIconIndex := -1;

  pAutoSize := not false;
  pIconSize := 40;
  ImagePathPrefix := TMain.Instance.FilePaths.SkinDirectory + '\BarInnerPanel';

  pIcons := TObjectList.Create();
  pIcons.OwnsObjects := false;

  self.OnMouseDown := Self_MouseDown;
end;


function TIconPanel.GetComponentByID(const componentID: Integer): TWComponent;
var i: Integer;
begin
	for i := 0 to pIcons.Count do begin
  	if TWComponent(pIcons[i]).ID = componentID then begin
      result := TWComponent(pIcons[i]);
      Exit;
    end;
  end;
  result := nil;
end;


function TIconPanel.CreateFormPopupMenu;
var menuItem: TMenuItem;
begin
  result := TPopupMenu.Create(self);

  menuItem := TMenuItem.Create(result);
  menuItem.Caption := TMain.Instance.Loc.GetString('IconPanel.PopupMenu.NewShortcut');
  menuItem.OnClick := MenuItem_Click_NewShortcut;
  result.Items.Add(menuItem);

  menuItem := TMenuItem.Create(result);
  menuItem.Caption := TMain.Instance.Loc.GetString('IconPanel.PopupMenu.NewSeparator');
  menuItem.OnClick := MenuItem_Click_NewSeparator;
  result.Items.Add(menuItem);
end;


procedure TIconPanel.MenuItem_Click_NewShortcut;
var folderItem: TFolderItem;
	component: TWComponent;
  filePath: String;
begin
	filePath := '';
  if OpenSaveFileDialog(Application.Handle, '', TMain.Instance.Loc.GetString('Global.OpenDialog.AllFiles') + '|*.*', '', TMain.Instance.Loc.GetString('IconPanel.NewShorcut.OpenDialog'), filePath, true) then begin
  	folderItem := TFolderItem.Create();
    folderItem.FilePath := TFolderItem.ConvertToRelativePath(filePath);
    folderItem.AutoSetName();
    TMain.Instance.User.AddFolderItem(folderItem);

    component := FolderItemToComponent(folderItem);
    AddChild(component);
    pIcons.Add(TObject(component));

    UpdateLayout();
  end;
end;


procedure TIconPanel.MenuItem_Click_NewSeparator;
var folderItem: TFolderItem;
	component: TWComponent;
begin
 	folderItem := TFolderItem.Create();
  folderItem.IsSeparator := true;
  TMain.Instance.User.AddFolderItem(folderItem);
  component := FolderItemToComponent(folderItem);
  AddChild(component);
  pIcons.Add(TObject(component));
  UpdateLayout();
end;


procedure TIconPanel.MenuItem_Click_AddItToQuickLaunch;
var component: TWComponent;
	menuItem: TMenuItem;
	folderItem: TFolderItem;
begin
	menuItem := Sender as TMenuItem;
	component := GetComponentByID(menuItem.Tag);
  folderItem := TMain.Instance.User.GetFolderItemByID(component.Tag);

  folderItem.QuickLaunch := not folderItem.QuickLaunch;
  TMain.Instance.User.InvalidateFolderItems();
end;


procedure TIconPanel.MenuItem_Click_Delete(Sender: TObject);
var component: TWComponent;
	menuItem: TMenuItem;
	answer: Integer;
begin
	answer := TMain.Instance.ConfirmationMessage(TMain.Instance.Loc.GetString('IconPanel.DeleteConfirmation'));
  if answer <> mrYes then Exit;

	menuItem := Sender as TMenuItem;

  component := GetComponentByID(menuItem.Tag);
  if component = nil then begin
  	elog('Couldn''t find component with ID: ' + IntToStr(menuItem.Tag));
    Exit;
  end;

  pIcons.Remove(TObject(component));

	component.Destroy();

  UpdateFolderItemsOrder();
  UpdateLayout();
end;


procedure TIconPanel.MenuItem_Click_Properties(Sender: TObject);
var folderItem: TFolderItem;
  component: TWComponent;
  hasChanged: Boolean;
begin
  component := GetComponentByID(TMenuItem(sender).Tag);
  folderItem := TMain.Instance.User.GetFolderItemByID(component.Tag);
 	hasChanged := TMain.Instance.User.EditFolderItem(folderItem);
	if hasChanged then begin
  	if component is TWFileIcon then begin
    	TWFileIcon(component).FilePath := folderItem.FilePath;
    end;
  end;
end;


procedure TIconPanel.Self_MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
	self.PopupMenu := CreateFormPopupMenu();
end;


procedure TIconPanel.SetAutoSize;
begin
  if pAutoSize = value then Exit;

	pAutoSize := value;
	UpdateLayout();
end;


procedure TIconPanel.iconForm_paint(Sender: TObject);
var rect:TRect;
begin
	if pIconDragData.IconForm = nil then Exit;

  if pIconDragData.Icon is TWFileIcon then begin
  	pIconDragData.IconForm.Canvas.Brush.Style := bsClear;
    TWFileIcon(pIconDragData.Icon).DrawToCanvas(pIconDragData.IconForm.Canvas, 0, 0);

		//pIconDragData.IconForm.Canvas.Draw(0, 0, TWFileIcon(pIconDragData.Icon).Icon);
  end else begin
  	rect.Top := 0;
    rect.Left := 0;
    rect.Bottom := pIconDragData.IconForm.Height;
    rect.Right := pIconDragData.IconForm.Width;
    pIconDragData.IconForm.Canvas.StretchDraw(rect, TWImage(pIconDragData.Icon).ImageObject);
  end;
end;


function TIconPanel.GetInsertionIndexAtPoint(const aPoint: TPoint; replacementBehavior: Boolean):Integer;
var p: TPoint;
	i :Integer;
	c: TWComponent;
begin
	result := 0;
  p := ScreenToClient(aPoint);

	if pIcons.Count = 0 then begin
  	result := 0;
    Exit;
  end;

  if p.X <= TWComponent(pIcons.Items[0]).Left + TWComponent(pIcons.Items[0]).Width / 2 then begin
  	result := 0;
    Exit;
  end;

  for i := 0 to pIcons.Count - 1 do begin
    c := TWComponent(pIcons.Items[i]);

    if replacementBehavior then begin

    	if i = pIcons.Count - 1 then begin
      	result := i;
        break;
      end;

      if (p.X >= c.Left) and (p.X < (c.Left + c.Width)) then begin
        result := i;
        break;
      end;

    end else begin

      if (p.X >= c.Left) and (p.X < c.Left + c.Width / 2) then begin
        result := i;
        break;
      end else begin
      	if i = pIcons.Count - 1 then begin
        	result := i + 1;
          break;
        end else begin
          if (p.X >= c.Left + c.Width / 2) and (p.X < c.Left + c.Width) then begin
            result := i + 1;
            break;
          end;
        end;
      end;

    end;

  end;

  if not pAutoSize then
  	if result > pLastVisibleIconIndex then result := pLastVisibleIconIndex;

end;


function TIconPanel.GetPotentialWidth;
var i, iconX, iconY, iconMaxX: Integer;
	component: TWComponent;
begin
	iconX := TMain.instance.Style.barInnerPanel.paddingLeft;
  iconY := TMain.instance.Style.barInnerPanel.paddingTop;
  iconMaxX := 0;

  for i := 0 to (pIcons.Count - 1) do begin
    component := TWComponent(pIcons[i]);

  	component.Left := iconX;
    component.Top := iconY;

    iconX := iconX + component.Width;

    iconMaxX := component.Left + component.Width;
  end;

  result := iconMaxX + TMain.Instance.Style.barInnerPanel.paddingRight;
end;


procedure TIconPanel.BrowseButtonPopupMenu_Click(Sender: TObject);
var folderItem: TFolderItem;
begin
	folderItem := TMain.Instance.User.GetFolderItemByID((Sender as TMenuItem).Tag);
  folderItem.Launch();
end;


procedure TIconPanel.BrowseButton_Click(Sender: TObject);
var pPopupMenu: TPopupMenu;
	menuItem: TMenuItem;
	i: Integer;
  folderItem: TFolderItem;
  component: TWComponent;
  mouse: TMouse;
  menuItemBitmap: TBitmap;
begin
  pPopupMenu := TPopupMenu.Create(self);

  for i := pLastVisibleIconIndex + 1 to pIcons.Count - 1 do begin
  	if i >= pIcons.Count then break;

    component := TWComponent(pIcons[i]);

    folderItem := TMain.Instance.User.GetFolderItemByID(component.Tag);

    menuItem := TMenuItem.Create(self);

    if folderItem.IsSeparator then begin
    	menuItem.Caption := '-';
      menuItem.Enabled := false;
    end else begin
    	menuItem.Caption := folderItem.Name;
      menuItem.Tag := folderItem.ID;

      if folderItem.SmallIcon <> nil then begin
      	menuItemBitmap := TBitmap.Create();
        menuItemBitmap.Width := 16;
        menuItemBitmap.Height := 16;
        menuItemBitmap.Canvas.Draw(0, 0, folderItem.SmallIcon);
        menuItem.OnClick := BrowseButtonPopupMenu_Click;
      	menuItem.Bitmap := menuItemBitmap;
      end;

    end;

    pPopupMenu.Items.Add(menuItem);
  end;

  mouse := TMouse.Create();

  pPopupMenu.Popup(mouse.CursorPos.X, mouse.CursorPos.Y);
end;


procedure TIconPanel.UpdateLayout();
var iconX, iconY: Word;
	i:Integer;
  icon: TWFileIcon;
  iconMaxX: Integer;
  folderItem: TFolderItem;
  component: TWComponent;
  potentialWidth: Integer;
begin
  iconX := TMain.instance.Style.barInnerPanel.paddingLeft;
  iconY := TMain.instance.Style.barInnerPanel.paddingTop;

  if pBrowseButton = nil then begin
    pBrowseButton := TWImageButton.Create(Owner);
    pBrowseButton.Visible := true;
    pBrowseButton.ImagePathPrefix := TMain.Instance.FilePaths.SkinDirectory + '\BrowseArrowButton';
    pBrowseButton.FitToContent();
    pBrowseButton.OnClick := BrowseButton_Click;
    AddChild(pBrowseButton);
  end;

  for i := 0 to (pIcons.Count - 1) do begin
  	if (i >= pIcons.Count) then break;
    component := TWComponent(pIcons[i]);
    component.Visible := false;
  end;

  for i := 0 to (pIcons.Count - 1) do begin
  	if (i >= pIcons.Count) then break;

    icon := TWFileIcon(pIcons[i]);

  	icon.Left := iconX;
    icon.Top := iconY;

    if not pAutoSize then begin
    	if icon.Left + icon.Width >= Width - pBrowseButton.Width then begin
      	pLastVisibleIconIndex := i - 1;
      	break;
      end;
    end;

    if icon is TWFileIcon then begin
      if icon.FilePath = '' then begin
        folderItem := TMain.Instance.User.GetFolderItemByID(icon.Tag);
        icon.FilePath := folderItem.ResolvedFilePath;
        icon.OverlayImageUpPath := TMain.Instance.FilePaths.SkinDirectory + '\IconOverlayUp.png';
        icon.OverlayImageDownPath := TMain.Instance.FilePaths.SkinDirectory + '\IconOverlayDown.png';
      end;
    end;

    if (pIconDragData.Started) and (pIconDragData.Icon = TWComponent(icon)) then begin
    end else begin
    	icon.Visible := true;
    end;

    iconX := iconX + icon.Width;

    iconMaxX := icon.Left + icon.Width;

    if not pAutoSize then begin
      if iconMaxX > Width then break;
    end;
  end;


  if pAutoSize then pLastVisibleIconIndex := pIcons.Count - 1;


  potentialWidth := self.PotentialWidth;

  if pAutoSize then
		Width := potentialWidth;
  Height := pIconSize + TMain.instance.Style.barInnerPanel.paddingV;


  if not pAutoSize then begin
    pBrowseButton.Visible := true;
    pBrowseButton.Left := Width - pBrowseButton.Width;
    pBrowseButton.Top := Round(Height / 2 - pBrowseButton.Height / 2);
  end else begin
  	pBrowseButton.Visible := false;
  end;


  if (potentialWidth > TMain.Instance.mainForm.IconPanelMaxWidth) and (AutoSize) then begin
  	Width := TMain.Instance.mainForm.IconPanelMaxWidth + 1;
    AutoSize := false;
  end else begin
  	if (potentialWidth <= TMain.Instance.mainForm.IconPanelMaxWidth) and (not AutoSize) then begin
    	AutoSize := true;
    end;
  end;
  
end;


function TIconPanel.FolderItemToComponent;
var icon: TWFileIcon;
	separatorImage: TWImage;
begin
  if not folderItem.IsSeparator then begin

    icon := TWFileIcon.Create(Owner);
    icon.Tag := folderItem.ID;
    icon.Width := pIconSize;
    icon.Height := pIconSize;
    icon.Visible := false;
    icon.OnMouseDown := icon_mouseDown;
    icon.OnMouseUp := icon_mouseUp;
    icon.OnClick := icon_click;
    icon.OnMouseEnter := Icon_MouseEnter;
    icon.OnMouseExit := Icon_MouseExit;

    result := TWComponent(icon);

  end else begin

    separatorImage := TWImage.Create(Owner);
    separatorImage.FilePath := TMain.Instance.FilePaths.SkinDirectory + '\InnerPanelSeparator.png';
    separatorImage.Visible := false;
    separatorImage.Tag := folderItem.ID;
    separatorImage.FitToContent();
    separatorImage.Height := pIconSize;
    separatorImage.StretchToFit := true;
    separatorImage.MaintainAspectRatio := false;
    separatorImage.OnMouseDown := icon_mouseDown;
    separatorImage.OnMouseUp := icon_mouseUp;

    result := TWComponent(separatorImage);

  end;
end;


procedure TIconPanel.LoadFolderItems();
var i: Integer;
  folderItem: TFolderItem;
  icon: TWFileIcon;
  component: TWComponent;
begin
	ilog('Creating icons');

	for i := 0 to (pIcons.Count - 1) do begin
  	if (i >= pIcons.Count) then break;
    icon := TWFileIcon(pIcons.Items[i]);
    icon.Free();
  end;

  pIcons.Clear();

  for i := 0 to TMain.instance.User.FolderItemCount - 1 do begin
  	folderItem := TMain.instance.User.getFolderItemAt(i);

    component := FolderItemToComponent(folderItem);

    AddChild(component);
    pIcons.Add(TObject(component));
  end;
end;


procedure TIconPanel.UpdateFolderItemsOrder();
var folderItemIDs: Array of Integer;
	i: Integer;
begin
	SetLength(folderItemIDs, pIcons.Count);
	for i := 0 to pIcons.Count - 1 do begin
  	folderItemIDs[i] := TWComponent(pIcons[i]).Tag;
  end;
  TMain.Instance.User.ReorderAndDeleteFolderItems(folderItemIDs);
end;


procedure TIconPanel.icon_mouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
	if Button <> mbLeft then Exit;  
  if not pIconDragData.Started then Exit;

  if pIconDragData.Timer <> nil then pIconDragData.Timer.Enabled := false;
  pIconDragData.Started := false;

  if pIconDragData.IconForm <> nil then FreeAndNil(pIconDragData.IconForm);
  if pIconDragData.Icon <> nil then pIconDragData.Icon.Visible := true;

  UpdateFolderItemsOrder();
end;


procedure TIconPanel.iconDragData_timer(Sender: TObject);
var mouse: TMouse;
  formMask: Graphics.TBitmap;
  rect: TRect;
  region: THandle;
  mouseOffset: TPoint;
  formCenter: Tpoint;
  indexUnderCursor: Integer;
  currentIndex: Integer;
  saveItem: TWComponent;
begin
	mouse := TMouse.Create();

  if not pIconDragData.Started then begin
  	if PointDistance(mouse.CursorPos, pIconDragData.MouseDownLoc) >= 5 then begin
    	ilog('Starting to drag icon...');
    	pIconDragData.Started := true;

      if pIconDragData.IconForm = nil then begin
      	pIconDragData.IconForm := TForm.Create(self);
        pIconDragData.IconForm.Visible := false;
        pIconDragData.IconForm.BorderStyle := bsNone;
        pIconDragData.IconForm.OnPaint := iconForm_paint;
        SetTransparentForm(pIconDragData.IconForm.Handle, 128);
      end;

      pIconDragData.Icon.Visible := false;

      if pIconDragData.Icon is TWFileIcon then begin

        formMask := Graphics.TBitmap.Create();

        try
          formMask.Width := pIconDragData.IconForm.Width;
          formMask.Height := pIconDragData.IconForm.Height;

          rect.Top := 0;
          rect.Left := 0;
          rect.Bottom := formMask.Height;
          rect.Right := formMask.Width;

          formMask.LoadFromFile(TMain.Instance.FilePaths.SkinDirectory + '/IconOverlayMask.bmp');

          region := CreateRegion(formMask);
          SetWindowRGN(pIconDragData.IconForm.Handle, region, True);
        finally
          formMask.Free();
        end;

      end;

      pIconDragData.IconForm.Width := pIconDragData.Icon.Width;
    	pIconDragData.IconForm.Height := pIconDragData.Icon.Height;

    end;
  end else begin

  	mouseOffset.X := mouse.CursorPos.X - pIconDragData.MouseDownLoc.X;
    mouseOffset.Y := mouse.CursorPos.Y - pIconDragData.MouseDownLoc.Y;

   	pIconDragData.IconForm.Left := pIconDragData.StartIconLoc.X + mouseOffset.X;
    pIconDragData.IconForm.Top := pIconDragData.StartIconLoc.Y + mouseOffset.Y;

    formCenter.X := pIconDragData.IconForm.Left + Round(pIconDragData.IconForm.Width / 2);
    formCenter.Y := pIconDragData.IconForm.Top + Round(pIconDragData.IconForm.Height / 2);

    if not (pIconDragData.Icon is TWFileIcon) then begin
    	formCenter.X := pIconDragData.IconForm.Left;
    end;


    indexUnderCursor := GetInsertionIndexAtPoint(formCenter, pIconDragData.Icon is TWFileIcon);
    currentIndex := pIcons.IndexOf(TObject(pIconDragData.Icon));

   	 
    
    if indexUnderCursor <> currentIndex then begin

    	if pIconDragData.Icon is TWFileIcon then begin

      	saveItem := TWComponent(pIcons.Items[indexUnderCursor]);
        pIcons.Items[indexUnderCursor] := TObject(pIconDragData.Icon);
        pIcons.Items[currentIndex] := saveItem;

      end else begin

      	if (indexUnderCursor >= pIcons.Count) and (currentIndex = pIcons.Count - 1) then begin

        end else begin

          pIcons.Remove(TObject(pIconDragData.Icon));

          if currentIndex > indexUnderCursor then begin
            pIcons.Insert(indexUnderCursor, TObject(pIconDragData.Icon));
          end else begin
            pIcons.Insert(indexUnderCursor-1, TObject(pIconDragData.Icon));
          end;

        end;

      end;
      
      UpdateLayout();
    end;



    pIconDragData.IconForm.Visible := true;

  end;

end;


procedure TIconPanel.icon_mouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var screenMouseLoc: TPoint;
	component: TWComponent;
  mouse: TMouse;
  popupMenu: TPopupMenu;
  menuItem: TMenuItem;
  folderItem: TFolderItem;
begin
	component := Sender as TWComponent;

  mouse := TMouse.Create();

  screenMouseLoc.X := mouse.CursorPos.X;
  screenMouseLoc.Y := mouse.CursorPos.Y;

	if Button = mbRight then begin
    ilog('Showing icon popup menu...');

    popupMenu := CreateFormPopupMenu();

    menuItem := TMenuItem.Create(popupMenu);
    menuItem.Caption := '-';
    menuItem.Enabled := false;
    popupMenu.Items.Add(menuItem);

    menuItem := TMenuItem.Create(popupMenu);
    menuItem.Caption := TMain.Instance.Loc.GetString('IconPanel.PopupMenu.Delete');
    menuItem.Tag := component.ID;
    menuItem.OnClick := MenuItem_Click_Delete;
    popupMenu.Items.Add(menuItem);

    if component is TWFileIcon then begin
    	folderItem := TMain.Instance.User.GetFolderItemByID(component.Tag);

      menuItem := TMenuItem.Create(popupMenu);
      menuItem.Caption := '-';
      menuItem.Enabled := false;
      popupMenu.Items.Add(menuItem);

      menuItem := TMenuItem.Create(popupMenu);
      menuItem.Caption := TMain.Instance.Loc.GetString('IconPanel.PopupMenu.AddToQuickLaunch');
      menuItem.Tag := component.ID;
      menuItem.Checked := folderItem.QuickLaunch;
      menuItem.OnClick := MenuItem_Click_AddItToQuickLaunch;
      popupMenu.Items.Add(menuItem);

      menuItem := TMenuItem.Create(popupMenu);
      menuItem.Caption := TMain.Instance.Loc.GetString('IconPanel.PopupMenu.Properties');
      menuItem.Tag := component.ID;
      menuItem.OnClick := MenuItem_Click_Properties;
      popupMenu.Items.Add(menuItem);
    end;

		popupMenu.Popup(screenMouseLoc.X, screenMouseLoc.Y);
  end else begin
  	ilog('Initializing drag data...');

  	if pIconDragData.Timer = nil then begin
      pIconDragData.Timer := TTimer.Create(self);
      pIconDragData.Timer.Interval := 50;
      pIconDragData.Timer.OnTimer := iconDragData_timer;
    end;

    pIconDragData.MouseDownLoc := screenMouseLoc;
    pIconDragData.Icon := component as TWComponent;
    pIconDragData.Started := false;
    pIconDragData.Timer.Enabled := true;

    pIconDragData.StartIconLoc := component.ScreenLoc;
  end;
end;


procedure TIconPanel.Icon_MouseEnter(sender: TObject);
var icon: TWFileIcon;
	folderItem: TFolderItem;
begin
	icon := TWFileIcon(sender);
  folderItem := TMain.Instance.User.GetFolderItemByID(icon.Tag);

  if pTooltipForm = nil then begin
  	pTooltipForm := TIconTooltipForm.Create(Self);
  end;
  pTooltipForm.ShowAbove(icon, folderItem.Name);
end;


procedure TIconPanel.Icon_MouseExit(sender: TObject);
begin
	pTooltipForm.Hide();
end;


function TIconPanel.IconCount():Word;
begin
	result := pIcons.Count;
end;


procedure TIconPanel.icon_click(sender: TObject);
var icon: TWFileIcon;
	folderItem: TFolderItem;
begin
  if pIconDragData.Started then Exit;

  pIconDragData.Timer.Enabled := false;

	icon := Sender as TWFileIcon;
  folderItem := TMain.Instance.User.GetFolderItemByID(icon.Tag);

  if folderItem = nil then begin
  	elog('No FolderItem with ID: ' + IntToStr(icon.Tag));
    Exit;
  end;

  ilog('Icon click: ' + folderItem.FilePath);

  folderItem.Launch(); 
end;


end.
