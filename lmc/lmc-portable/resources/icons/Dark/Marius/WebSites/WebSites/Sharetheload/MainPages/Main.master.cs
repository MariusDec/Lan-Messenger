using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class Main : System.Web.UI.MasterPage
{
    protected void Page_Load(object sender, EventArgs e)
    {
        
    }
    protected void Unnamed_Click(object sender, EventArgs e)
    {
        /*UserAccount loggedUser = (UserAccount)Session["loggedUser"];
        if (loggedUser == null)
            return;*/

        String produs = SearchButton.Text;

        List<Product> listaProduse = ProductWrapper.getByName(produs);
        foreach (Product prod in listaProduse)
        {
            Console.Write(prod.Name);
        }

        List<Product> listaProduseDescription = ProductWrapper.getByDescription(produs);
        foreach (Product prod in listaProduseDescription)
        {
            Console.Write(prod.Name);
        }

        List<Request> listaRequests = RequestWrapper.getName(produs);
        foreach (Request req in listaRequests)
        {
            Console.Write(req.Title);
        }
        StorageManager.lastSearchedRequests = listaRequests;
        Server.Transfer("SearchResultPage.aspx", true);
    }


    protected void logoutButton_Click(object sender, EventArgs e)
    {
        Session["UserLogged"] = null;
        Server.Transfer("../Account/Login.aspx", true);
    }
}
