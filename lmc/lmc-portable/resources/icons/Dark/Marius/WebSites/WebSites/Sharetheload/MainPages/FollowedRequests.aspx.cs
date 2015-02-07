using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class Pages_FallowedRequests : System.Web.UI.Page
{
    List<Controls_RequestControl> controls;
    protected void Page_Load(object sender, EventArgs e)
    {

        if (!this.IsPostBack)
        {
            InitializeFields();
        }
    }

    private void InitializeFields()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        List<Suscription> allRequests = SubscriptionWrapper.getByUserId(((UserAccount)Session["loggedUser"]).UserID);

        int count = allRequests.Count;
        controls = new List<Controls_RequestControl>(count);
        table.Controls.Clear();

        for (int index = 0; index < count; index++)
        {
            Suscription suscription = allRequests[index];
            TableRow rowNew = new TableRow();
            table.Controls.Add(rowNew);

            var reqID = suscription.RequestID;
            Request request = RequestWrapper.getById(reqID);
            TableCell cellNew = new TableCell();
            
            //Label lblNew = new Label();
            //lblNew.Text = request.Title + " " + request.StartDate + " " + request.EndDate + " "
            //                        + request.MinUsers + " " + request.MaxUsers + " " + request.City + " "
            //                        + request.State + " " + request.Sector + " " + request.Street + " "
            //                        + request.Status + "<br />";

            UserControl uc = (UserControl)Page.LoadControl("../Controls/RequestControl.ascx");
            ((Controls_RequestControl)uc).Request = request;
            Button subscribeButton = (Button)((Controls_RequestControl)uc).FindControl("subscribeButton");
            subscribeButton.Visible = false;
            controls.Add((Controls_RequestControl)uc);
            cellNew.Controls.Add((Controls_RequestControl)uc);
            rowNew.Controls.Add(cellNew);
        }
    }
}