using Microsoft.AspNet.Identity;
using Microsoft.Owin.Security;
using System;
using System.Web;
using System.Web.UI;
using Sharetheload;
using System.Web.UI.WebControls;

public partial class Account_Login : Page
{
        protected void Page_Load(object sender, EventArgs e)
        {
            if (!this.IsPostBack)
            {
                RegisterHyperLink.NavigateUrl = "Register";
                //OpenAuthLogin.ReturnUrl = Request.QueryString["ReturnUrl"];
                var returnUrl = HttpUtility.UrlEncode(Request.QueryString["ReturnUrl"]);
                if (!String.IsNullOrEmpty(returnUrl))
                {
                    RegisterHyperLink.NavigateUrl += "?ReturnUrl=" + returnUrl;
                }
            }
        }

        private UserAccountsWrapper userAccount = new UserAccountsWrapper();

        private bool isPasswordValid = false;
        private bool isUserValid = false;

        protected void LogIn(object sender, EventArgs e)
        {
            if (isPasswordValid && isUserValid)
            {
                string userName = textBoxUserName.Text.ToString();
                string password = textBoxPassword.Text.ToString();
                Session["loggedUser"] = userAccount.getUser(userName, password);
                Server.Transfer("../MainPages/Main.aspx");
            }
        }


        protected void PasswordValidate(object source, ServerValidateEventArgs args)
        {
            if (isUserValid)
            {
                string userName = textBoxUserName.Text.ToString();
                string password = textBoxPassword.Text.ToString();
                isPasswordValid = userAccount.authentificateUser(userName, password);
                args.IsValid = isPasswordValid;
            }
        }

        protected void UserValidate(object source, ServerValidateEventArgs args)
        {
            string userName = textBoxUserName.Text.ToString();
            isUserValid = userAccount.checkIfUserExist(userName);
            args.IsValid = isUserValid;
        }
}