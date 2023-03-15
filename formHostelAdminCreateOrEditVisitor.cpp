//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "dataModuleHostAdmin.h"
#include "Person.h"
#include "formHostelAdminCreateOrEditVisitor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "DBGridEh"
#pragma link "DBGridEhGrouping"
#pragma link "GridsEh"
#pragma link "JvDBLookupTreeView"
#pragma link "JvExControls"
#pragma link "DBGridEh"
#pragma link "DBGridEhGrouping"
#pragma link "GridsEh"
#pragma link "JvDBLookupTreeView"
#pragma link "JvExControls"
#pragma resource "*.dfm"
TFHostelAdminCreateOrEditVisitor *FHostelAdminCreateOrEditVisitor;
//---------------------------------------------------------------------------
__fastcall TFHostelAdminCreateOrEditVisitor::TFHostelAdminCreateOrEditVisitor(TComponent* Owner, TPerson* APerson)
	 : TForm(Owner), person_(APerson), isCanClose(true)
{
	 g_dataModuleHostelAdmin->qrPersonInfo->AfterScroll = outPutFullNameAfterScroll;
	 person_->personId == -1 ? this->prepareForCreate() : this->prepareForEdit();
}

//---------------------------------------------------------------------------
void __fastcall TFHostelAdminCreateOrEditVisitor::ePersonIdChange(TObject *Sender)
{
	 if(g_dataModuleHostelAdmin->qrPersonInfo->Active) {

		  TEdit* edit = static_cast<TEdit*>(Sender);

		  String txt = edit->Text, filterText = "";
		  int personId = -1;
		  if (!txt.IsEmpty()) {
			   if (TryStrToInt(txt, personId)) {
					filterText = "personId_str LIKE '%" + IntToStr(personId) + "%'";
			   }
			   else {
					filterText = "fullName LIKE '%" + txt  + "%'";
			   }

			   g_dataModuleHostelAdmin->qrPersonInfo->Filter = filterText;
			   g_dataModuleHostelAdmin->qrPersonInfo->Filtered = true;
		  }
		  else {
			   g_dataModuleHostelAdmin->qrPersonInfo->Filtered = false;
		  }
	 }
}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::dtFromCloseUp(TObject *Sender)
{
	 TDateTimePicker* dtFrom = static_cast<TDateTimePicker*>(Sender);

	 if (!(dtFrom->DateTime >= Now())) {
		  dtFrom->Date = Now();
	 }

	 else if (dtFrom->Date >= dtTo->Date ) {
			  dtFrom->Date = dtTo->Date - 1;
	 }

	 if (person_->personId != -1) {
		  person_->dateFrom = dtFrom->DateTime;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::dtToCloseUp(TObject *Sender)
{
	 TDateTimePicker* dtTo = static_cast<TDateTimePicker*>(Sender);

	if (!(dtTo->DateTime >= Now() )) {
		  dtTo->Date = Now() + 1;
	 }

	 if (&TFHostelAdminCreateOrEditVisitor::prepareForEdit) {

		 if (dtTo->Date == dtFrom->Date) {
			dtTo->Date = dtFrom->Date;
			}
		else
		   { if (dtTo->Date <= dtFrom->Date ) {
			 dtFrom->Date = dtTo->Date - 1;
		   }
		 }
	 }

	 if (person_->personId  != -1) {
		  person_->dateTo = dtTo->DateTime;
	 }

}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::prepareForCreate() {
	 dtFrom->Date = Now();
	 dtTo->Date = Now() + 1;


	 labEmploysFIO->Caption = "";
	 this->Caption = this->Caption + " �������� ������ ����������";


}

//---------------------------------------------------------------------------
void __fastcall TFHostelAdminCreateOrEditVisitor::prepareForEdit() {
	 DBGridEh1->Visible = false;
	 gpProperties->Align = alClient;
	 Constraints->MaxHeight = 300;

	 cbHostel->Enabled = false;
	 ePersonId->Enabled = false;


	 dtFrom->DateTime = person_->dateFrom;
	 dtTo->DateTime = person_->dateTo;
	 ePersonId->Text = IntToStr(person_->personId);
	 cbHostel->KeyValue = person_->hostelId;


	 this->Caption = this->Caption + " �������� �������� ����������";
}

//---------------------------------------------------------------------------
void __fastcall TFHostelAdminCreateOrEditVisitor::eNumbOfDaysChange(TObject *Sender)
{
	 TEdit* edit = static_cast<TEdit*>(Sender);
	 bool isEmpty = edit->Text.IsEmpty();
	 if (!isEmpty) {
		  this->dtTo->Date = this->dtFrom->Date + StrToInt(edit->Text);
	 }
	 else {
		  this->dtTo->Date = this->dtFrom->Date;

	 }
	 dtTo->Enabled = isEmpty;
}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::dtFromChange(TObject *Sender)
{
	 if (!eNumbOfDays->Text.IsEmpty()) {
		  this->dtTo->Date = this->dtFrom->Date + StrToInt(eNumbOfDays->Text);
	 }
}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::outPutFullNameAfterScroll(TDataSet *DataSet) {
	 if (DataSet->Active && DataSet->RecordCount > 0) {
		  labEmploysFIO->Caption = DataSet->FieldValues["fullName"];
	 }
}
void __fastcall TFHostelAdminCreateOrEditVisitor::FormClose(TObject *Sender, TCloseAction &Action)
{
	 g_dataModuleHostelAdmin->qrPersonInfo->AfterScroll = NULL;
	 g_dataModuleHostelAdmin->qrPersonInfo->Filtered = false;
}
//---------------------------------------------------------------------------
bool __fastcall TFHostelAdminCreateOrEditVisitor::chechBeforeSave() {
	 //�������� �� ����� ���������
	 if (cbHostel->KeyValue.IsNull()) {
		  ShowMessage("�������� ���������!");
		  return false;
	 }

	 //��������� ���������� ������ 1 ������, ������ "���. � (���)"
	 if (ePersonId->Text.IsEmpty()) {
		  ShowMessage("��������� \"���. � (���)\"");
		  return false;
	 }

	 std::unique_ptr<TADOQuery> pCheck(new TADOQuery(0));
	 pCheck->Connection = g_dataModuleHostelAdmin->ADOConnectionToOffice;

	 String personId = g_dataModuleHostelAdmin->qrPersonInfo->FieldByName("personId_str")->AsString;
	 TDateTime tdTo = dtTo->Date;
	 String tdFrom = dtFrom->Date.DateTimeString();
	 String query;

	 if (person_->personId == -1) {
		  //��������� ��������� �� ��� ������ ��������� � ����� � ��������� ����
		  query =
			   " SELECT 1 "
			   " FROM Books.dbo.tblPersonHostelSchedule "
			   " WHERE DateTo >= CAST('" + tdFrom +"' AS DATE) "
			   "     AND ID_Person = " + personId;

		  pCheck->SQL->Text = query;
		  pCheck->Open();
		  if (pCheck->Active && pCheck->RecordCount > 0) {
			   String msg = "�� ��������� ���� ��������� " + labEmploysFIO->Caption +
					" ��� ��������� � ���������. �������� ���� ���������� ��� �������� ��� ������������ ������";
			   ShowMessage(msg);
			   return false;
		  }
	 }


	 //������� ������ � ���� � ���������� � ��������� ������������
	 query =
		  " SELECT 1 "
		  " FROM Personal.dbo.fnGetScheduleHistForPerson(" + personId + ")"
		  " WHERE ID_Schedule NOT IN (19) "
		  " 	AND (ToDate >= CAST('" + tdFrom +"' AS DATE) OR ToDate IS NULL) "
		  " ORDER BY FromDate DESC ";

	 pCheck->Close();
	 pCheck->SQL->Text = query;
	 pCheck->Open();
	 if (pCheck->Active && !pCheck->RecordCount > 0) {
		  String msg = "������! ���� ���������� � ��������� �� ������������ � ������� ��������";
		  ShowMessage(msg);
		  return false;
	 }

	 //���� �� � ���������� ����������� � ������� 5 ����� (-5000)
	 query =
		  " DECLARE "
		  " 	@dateFrom DATE = CAST(GETDATE() - DAY(GETDATE()) + 1 AS DATE), "
		  " 	@dateTo DATE = EOMONTH(GETDATE()) "
		  " SELECT "
		  " 	ID_Person, "
		  "     SUM(SummaPay) AS Pay "
		  " FROM Personal.dbo.fnGetPersonZP(" + personId + ", @dateFrom,@dateTo) "
		  " GROUP BY ID_Person ";

	 pCheck->Close();
	 pCheck->SQL->Text = query;
	 pCheck->Open();
	 if (pCheck->Active && pCheck->RecordCount > 0) {
		  if (pCheck->FieldValues["Pay"] < -5000) {
			   String msg = "� ���������� " + labEmploysFIO->Caption + " ���� ����������� � ������� "
			   + pCheck->FieldByName("Pay")->AsString + " �� ����� �������� � ���������?";
			   if (Application->MessageBox(msg.w_str(), L"��������������", MB_YESNO | MB_ICONINFORMATION) != IDYES) { return false; }
		  }
	 }

	 //���� �� ����������� �� �������� ���������
	 query =
		  " SELECT DateFrom, DateTo "
		  " FROM Books.dbo.tblPersonHostelSchedule "
		  " WHERE ID_Person = " + personId +
		  " 	AND ID_Payment IS NULL";

	 pCheck->Close();
	 pCheck->SQL->Text = query;
	 pCheck->Open();
	 if (pCheck->Active && pCheck->RecordCount > 0) {
		  String msg = "��������� �� ������� ���������� � ���������:\n";
		  String interval = "";
		  for (pCheck->First(); !pCheck->Eof; pCheck->Next()) {
			   interval += "� " + pCheck->FieldByName("DateFrom")->AsString
					+ " �� " +  pCheck->FieldByName("DateTo")->AsString + "\n";
		  }
		  msg += interval;
		  msg += "�� ����� ��������?";
		  if (Application->MessageBox(msg.w_str(), L"��������������", MB_YESNO | MB_ICONINFORMATION) != IDYES) { return false; }
	 }

	 query =
		  " SELECT FromDate, ToDate, DATEDIFF(dd, ToDate, CAST('" + tdTo.DateString() + "' AS DATE)) AS Diff"
		  " FROM Personal.dbo.fnGetScheduleHistForPerson(" + personId + ")"
		  " WHERE ID_Schedule NOT IN (19) "
		  " 	AND ToDate >= CAST('" + tdFrom +"' AS DATE) "
		  " ORDER BY FromDate DESC ";

	 pCheck->Close();
	 pCheck->SQL->Text = query;
	 pCheck->Open();
	 if (pCheck->Active && pCheck->RecordCount > 0) {
		  if (pCheck->FieldByName("ToDate")->AsDateTime < tdTo) {
			   String msg = "��������� " + labEmploysFIO->Caption + " ����� ���� � ��������� �� "
			   + pCheck->FieldByName("Diff")->AsString + " ���� ������, ��� ������ ��� ������� ������, �� �������???";
			   if (Application->MessageBox(msg.w_str(), L"��������������", MB_YESNO | MB_ICONINFORMATION) != IDYES) { return false; }
		  }
	 }


	 return true;
}

void __fastcall TFHostelAdminCreateOrEditVisitor::btnSaveClick(TObject *Sender)
{
	 this->isCanClose = chechBeforeSave();

	 if (isCanClose) {

		  if (person_->personId == -1) {
			   person_->personId = g_dataModuleHostelAdmin->qrPersonInfo->FieldValues["ID_Person"];
			   person_->hostelId = cbHostel->KeyValue;
		  }

		  person_->fullName = labEmploysFIO->Caption;
		  person_->dateFrom = dtFrom->Date;

		  if (dtTo->Enabled) {
			   person_->dateTo = dtTo->Date;
		  }
		  else {
			   person_->dateTo = dtFrom->Date + StrToInt(eNumbOfDays->Text);
		  }
	 }
}
//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::FormCloseQuery(TObject *Sender,
		  bool &CanClose)
{
	 //���� � ��� �� ����� ���������� ������� ������, �� �� ��������� ���� � ���������� � true
	 //��� ���� ����� ����� ����� ���� ������� ������� ��������� ("������", "�������" � �.�.)
	 CanClose = this->isCanClose;
	 this->isCanClose = true;
}
//---------------------------------------------------------------------------


void __fastcall TFHostelAdminCreateOrEditVisitor::dtFromKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift)
{
	TDateTimePicker* dtFrom = static_cast<TDateTimePicker*>(Sender);

	if (!(dtFrom->DateTime >= Now())) {
		  dtFrom->Date = Now();
	 }

	 else if (dtFrom->Date >= dtTo->Date ) {
			  dtFrom->Date = dtTo->Date - 1;
	 }

	 if (person_->personId != -1) {
		  person_->dateFrom = dtFrom->DateTime;
	 }
}

//---------------------------------------------------------------------------

void __fastcall TFHostelAdminCreateOrEditVisitor::dtToKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift)
{
	TDateTimePicker* dtTo = static_cast<TDateTimePicker*>(Sender);

	 if (!(dtTo->DateTime >= Now() )) {
		  dtTo->Date = Now() + 1;
	 }
	 if (&TFHostelAdminCreateOrEditVisitor::prepareForEdit) {

		 if (dtTo->Date == dtFrom->Date) {
			dtTo->Date = dtFrom->Date;
			}

		else
			{ if(dtTo->Date <= dtFrom->Date ) {
			 dtFrom->Date = dtTo->Date - 1;
			}
		}
	}

	 if (person_->personId  != -1) {
		  person_->dateTo = dtTo->DateTime;
	 }
}
//---------------------------------------------------------------------------

