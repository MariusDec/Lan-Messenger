using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class Pages_MyRequests : System.Web.UI.Page
{
    List<Controls_RequestControl> controls;

    protected void Page_Load(object sender, EventArgs e)
    {
        if (Session["loggedUser"] == null)
        {
            Server.Transfer("../Account/Login.aspx");
        }
        else
        {
            if (!this.IsPostBack)
            {
                InitializeFields();
            }
        }
    }

    private void InitializeFields()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        List<Request> allRequests = RequestWrapper.getAllRequests(((UserAccount)Session["loggedUser"]).UserID);

        int count = allRequests.Count;
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

                UserControl uc = (UserControl)Page.LoadControl("../Controls/RequestControl.ascx");
                ((Controls_RequestControl)uc).Request = req;
                Button subscribeButton = (Button)((Controls_RequestControl)uc).FindControl("subscribeButton");
                subscribeButton.Visible = false;
                controls.Add((Controls_RequestControl)uc);
                cellNew.Controls.Add((Controls_RequestControl)uc); 
                rowNew.Controls.Add(cellNew);
            }
    }
}