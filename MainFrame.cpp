#include "MainFrame.h"
#include "Constants.h"
#include "wx/dcbuffer.h"


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_PAINT(MainFrame::OnPaint)
  //EVT_LEFT_DOWN(MainFrame::OnMouseDown)
  //EVT_LEFT_UP(MainFrame::OnMouseUp)
  //EVT_MOTION(MainFrame::OnMouseMove)
END_EVENT_TABLE()


MainFrame::MainFrame()
: wxFrame(
  (wxFrame *)NULL,
  wxID_ANY,
  wxEmptyString,
  wxDefaultPosition,
  wxDefaultSize,
  0 | wxFRAME_SHAPED | wxSIMPLE_BORDER
  ) // 0 | wxFRAME_SHAPED | wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP
{
  // Necessary because we are going to do the drawing ourselves
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  
  // Load the mask and background images
  pMaskNineSlices.LoadImage(wxT("Data/Skin/Default/BarBackgroundRegion.png"));
  pBackgroundNineSlices.LoadImage(wxT("Data/Skin/Default/BarBackground.png"));

  pResizerPanel = new ImagePanel(this, wxID_ANY, wxPoint(0, 0), wxSize(50, 50));
  //pResizerPanel->LoadImage(wxT("Data/Skin/Default/Resizer.png"));
  //pResizerPanel->FitToContent();

  pWindowDragData.DraggingStarted = false;

  pBackgroundPanel = new NineSlicesPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(200, 200));
  pBackgroundPanel->LoadImage(wxT("Data/Skin/Default/BarBackground.png"));

  pBackgroundPanel->Connect(wxID_ANY, wxEVT_LEFT_DOWN, wxMouseEventHandler(MainFrame::OnMouseDown), NULL, this);
  pBackgroundPanel->Connect(wxID_ANY, wxEVT_LEFT_UP, wxMouseEventHandler(MainFrame::OnMouseUp), NULL, this);
  pBackgroundPanel->Connect(wxID_ANY, wxEVT_MOTION, wxMouseEventHandler(MainFrame::OnMouseMove), NULL, this);

  UpdateMask();
  UpdateLayout();
} 


void MainFrame::UpdateMask() {
  // Create the bitmap on which the 9-slices scaled mask is going to be drawn
  wxBitmap maskBitmap = wxBitmap(GetClientSize().GetWidth(), GetClientSize().GetHeight());
  
  // Create a temporary DC to do the actual drawing and assign it the bitmap
  wxMemoryDC maskDC;
  maskDC.SelectObject(maskBitmap);
  
  // Draw the nine slices on the DC
  pMaskNineSlices.Draw(&maskDC, 0, 0, maskBitmap.GetWidth(), maskBitmap.GetHeight());

  // Select NULL to release the bitmap
  // TODO: Should the bitmap be manually deleted at this point or is it handled by wxWidgets?
  maskDC.SelectObject(wxNullBitmap);

  // Create the region from the bitmap and assign it to the window
  wxRegion region(maskBitmap, lcMASK_COLOR);
  SetShape(region);
}


void MainFrame::UpdateLayout() {
  pBackgroundPanel->SetSize(0, 0,
    GetClientSize().GetWidth(),
    GetClientSize().GetHeight()
  );
}


void MainFrame::OnPaint(wxPaintEvent &evt) {
  wxBufferedPaintDC dc(this);

}


void MainFrame::OnMouseDown(wxMouseEvent& evt) {
  static_cast<wxWindow*>(evt.GetEventObject())->CaptureMouse();

  pWindowDragData.DraggingStarted = true;
  pWindowDragData.InitMousePos = ClientToScreen(evt.GetPosition());
  pWindowDragData.InitWindowPos = GetPosition();
}


void MainFrame::OnMouseUp(wxMouseEvent& evt) {
  wxWindow* w = static_cast<wxWindow*>(evt.GetEventObject());
  if (w->HasCapture()) w->ReleaseMouse();
  pWindowDragData.DraggingStarted = false;
}


void MainFrame::OnMouseMove(wxMouseEvent& evt) {
  if (pWindowDragData.DraggingStarted && evt.Dragging() && evt.LeftIsDown()) {
    wxPoint mousePos = ClientToScreen(evt.GetPosition());
    wxPoint mouseOffset = mousePos - pWindowDragData.InitMousePos;
    Move(mouseOffset.x + pWindowDragData.InitWindowPos.x, mouseOffset.y + pWindowDragData.InitWindowPos.y);
  }
}