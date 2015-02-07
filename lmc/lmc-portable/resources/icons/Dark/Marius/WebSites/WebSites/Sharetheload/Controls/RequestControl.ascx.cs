using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class Controls_RequestControl : System.Web.UI.UserControl
{
    private Request request = new Request();
    public Request Request
    {
        get { return request; }
        set { request = value; }
    }
    protected void Page_Load(object sender, EventArgs e)
    {
        Product prod = ProductWrapper.getById(request.ProductID);

        this.titleLabel.Text = request.Title;
        this.descriptionLabel.Text = prod.Description;
        this.startDate.Text = request.EndDate.ToString();
        this.endDate.Text = request.EndDate.ToString();

        List<Suscription> subs = SubscriptionWrapper.getByRequestId(request.ID);
        this.subscribers.Text = subs.Count.ToString();
    }
    protected void subscribeButton_Click(object sender, EventArgs e)
    {
        SubscriptionWrapper sub = new SubscriptionWrapper();
        sub.RequestID = request.ID;
        sub.UserID = ((UserAccount)Session["loggedUser"]).UserID;

        sub.Save();
    }
}