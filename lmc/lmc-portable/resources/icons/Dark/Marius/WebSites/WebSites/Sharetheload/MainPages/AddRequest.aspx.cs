using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.Services;

public partial class AddRequest : System.Web.UI.Page
{
    protected void Page_Load(object sender, EventArgs e)
    {
        if (Session["loggedUser"] == null)
        {
            Server.Transfer("/Account/Login.aspx");
        }
        else
        {
            if (!Page.IsPostBack)
            {
                Session["latitude"] = ((UserAccount)Session["loggedUser"]).Latitude;
                Session["longitude"] = ((UserAccount)Session["loggedUser"]).Longitude;
                Session["radius"] = 5000;
                Session["radiusChanged"] = false;
            }
        }
    }    

    protected void AddRequest_Click(object sender, EventArgs e)
    {
        ProductWrapper newProd = new ProductWrapper();
        newProd.Name = ProductName.Text;
        newProd.Category = Enum.GetName(typeof(EnumCategory), CategoryList.SelectedIndex);
        newProd.Description = ProductDesc.Text;
        newProd.Price = Price.Text;

        newProd.Save();

        RequestWrapper newReq = new RequestWrapper();
        newReq.Title = RequestTitle.Text;
        newReq.UserID = ((UserAccount)Session["loggedUser"]).UserID;
        newReq.ProductID = newProd.ID;
        newReq.StartDate = DateTime.Now;
        newReq.EndDate = datePickerDeadline.Now; //new DateTime(hiddenFieldDate.value);
        newReq.Latitude = ((UserAccount)Session["loggedUser"]).Latitude;
        newReq.Longitude = ((UserAccount)Session["loggedUser"]).Longitude;
        int radius;
        Int32.TryParse((Request.Cookies["Radius"]).Value, out radius);
        newReq.Radius = radius;

        int minVal, maxVal;
        Int32.TryParse(MinFollow.Text, out minVal);
        Int32.TryParse(MaxFollow.Text, out maxVal);
        newReq.MinUsers = minVal;
        newReq.MaxUsers = maxVal;
        newReq.Status = EnumRequestStatus.InProgress.ToString();

        newReq.Save();

        Server.Transfer("/Pages/MyRequests.aspx");
    }

}