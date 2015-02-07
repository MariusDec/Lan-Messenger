using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class Main : System.Web.UI.Page
{
    List<Controls_RequestControl> controls;
    protected void Page_Load(object sender, EventArgs e)
    {
       // if (!this.IsPostBack && ((UserAccount)Session["loggedUser"]) != null)
       // {
            InitializeFields();
       // }
    }

    private void InitializeFields()
    {
        DataClassesDataContext dc = new DataClassesDataContext();

        Double latitude;
        Double longitude;

        string valueLat = ((UserAccount)Session["loggedUser"]).Latitude;
        string valueLong = ((UserAccount)Session["loggedUser"]).Longitude;
        var style = NumberStyles.Number | NumberStyles.AllowCurrencySymbol;
        var culture = CultureInfo.CreateSpecificCulture("en-GB");
        Double.TryParse(valueLat, style, culture, out latitude);
        Double.TryParse(valueLong, style, culture, out longitude);

        List<Request> allRequests = RequestWrapper.GetAllInRange(((UserAccount)Session["loggedUser"]).UserID, latitude, longitude);

        int count = (allRequests.Count < 10) ? allRequests.Count : 10;
        controls = new List<Controls_RequestControl>(count);

        table.Controls.Clear();
        for (int index = 0; index < count; index++)
        {
            Request req = allRequests[index];
            TableRow rowNew = new TableRow();
            table.Controls.Add(rowNew);

            TableCell cellNew = new TableCell();

            //Label lblNew = new Label();
            //lblNew.Text = req.Title + " " + req.StartDate + " " + req.EndDate + " "
            //                    + req.MinUsers + " " + req.MaxUsers + " " + req.City + " "
            //                    + req.State + " " + req.Sector + " " + req.Street + " "
            //                    + req.Status + "<br />";

            UserControl uc = (UserControl)Page.LoadControl("/Controls/RequestControl.ascx");
            ((Controls_RequestControl)uc).Request = req;
            controls.Add((Controls_RequestControl)uc);
            cellNew.Controls.Add((Controls_RequestControl)uc);
            rowNew.Controls.Add(cellNew);
        }
    }
}