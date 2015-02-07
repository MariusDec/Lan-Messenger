using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class SearchResultPage : System.Web.UI.Page
{
    List<Controls_RequestControl> controls;
    protected void Page_Load(object sender, EventArgs e)
    {
        if (!this.IsPostBack)
        {
            InitializeFields(StorageManager.lastSearchedRequests);
        }
    }
        private void InitializeFields(List<Request> allRequests)
        {
            int count = (allRequests.Count < 10) ? allRequests.Count : 10;
            controls = new List<Controls_RequestControl>(count);

            table.Controls.Clear();
            for (int index = 0; index < count; index++)
            {
                Request req = allRequests[index];
                TableRow rowNew = new TableRow();
                table.Controls.Add(rowNew);
                TableCell cellNew = new TableCell();
                UserControl uc = (UserControl)Page.LoadControl("/Controls/RequestControl.ascx");
                ((Controls_RequestControl)uc).Request = req;
                controls.Add((Controls_RequestControl)uc);
                cellNew.Controls.Add((Controls_RequestControl)uc);
                rowNew.Controls.Add(cellNew);
            }
        }
    
}