unit WButtonBase;

interface

uses WComponent, Controls, Classes;

type

	TWButtonBase = class(TWComponent)

  	protected

      procedure Click; override;
      procedure MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override;
      procedure MouseEnter(); override;
      procedure MouseLeave(); override;
      procedure MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer); override; 
      procedure MouseMove(Shift: TShiftState; X, Y: Integer); override;
      
  end;


implementation


uses Logger;


procedure TWButtonBase.MouseDown(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  inherited;
  Invalidate();
end;


procedure TWButtonBase.Click();
begin
	inherited;
  //ilog('CLICK');
  Invalidate();
end;


procedure TWButtonBase.MouseUp(Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  inherited;
	//ilog('UP');
  Invalidate();
end;


procedure TWButtonBase.MouseMove(Shift: TShiftState; X, Y: Integer);
begin
  inherited;

end;


procedure TWButtonBase.MouseEnter();
begin
	inherited;
  //ilog('ENTER');
  Invalidate();
end;


procedure TWButtonBase.MouseLeave();
begin
	inherited;
  //ilog('LEAVE');
  Invalidate();
end;

end.
