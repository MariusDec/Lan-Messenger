using Microsoft.AspNet.Identity;
using System;
using System.Linq;
using System.Web.UI;
using Sharetheload;

using System.ComponentModel;
using System.Web.UI.WebControls;
using System.Net.Mail;
using System.Web.Services;
using System.Web;

public partial class Account_Register : Page
{
    private static Double latitude;
    private static Double longitude;
    protected void CreateUser_Click(object sender, EventArgs e)
    {
        if (isUserAlreadyCreated)
            return;
        Address userAddress = new Address();
        userAddress.Country = textBoxCountry.Text;
        userAddress.State = textBoxState.Text;
        userAddress.City = textBoxCity.Text;

        UserAccount newUser = new UserAccount();
        newUser.FirstName = textBoxFirstName.Text;
        newUser.LastName = textBoxLastName.Text;
        newUser.Email = textBoxEmail.Text;
        newUser.Password = textBoxPassword.Text;
        newUser.Phone = textBoxPhone.Text;
        newUser.AddressID = (new AddressWrapper()).Save(userAddress);
        HttpCookie latitCookie = Request.Cookies["Latitude"];
        HttpCookie longCookie = Request.Cookies["Longitude"];
        newUser.Latitude = latitCookie.Value;
        newUser.Longitude = longCookie.Value;
        (new UserAccountsWrapper()).Save(newUser);

        SmtpClient client = new SmtpClient();
        MailAddress destination = new MailAddress(newUser.Email.ToString());
        MailMessage message = new MailMessage();
        message.To.Add(destination);
        message.Body = "Hello Mr " +newUser.FirstName + ",";
        message.Body += Environment.NewLine + "Thank you for registering your ShareTheLoad account.";
        message.Body += Environment.NewLine;
        message.Body += Environment.NewLine;
        message.Body += Environment.NewLine + "Regards,";
        message.Body += Environment.NewLine + "Your ShareTheLoad Team";
        message.Body += Environment.NewLine + "Contact : .....";
        message.BodyEncoding = System.Text.Encoding.UTF8;
        message.Subject = "Share the load account - E-Mail Confirmation";
        message.SubjectEncoding = System.Text.Encoding.UTF8;
        client.SendCompleted += new SendCompletedEventHandler(SendCompletedCallback);
        client.Send(message);
        
        Server.Transfer("../Account/Login.aspx");

    }
    static bool mailSent = false;
    private static void SendCompletedCallback(object sender, AsyncCompletedEventArgs e)
    {
        // Get the unique identifier for this asynchronous operation.
        String token = (string)e.UserState;
        if (e.Error != null)
        {
            mailSent = false;
        }
        mailSent = true;

    }
    bool isUserAlreadyCreated = false;
    protected void EmailValidate(object source, ServerValidateEventArgs args)
    {

        string userEmail = textBoxEmail.Text.ToString();
        isUserAlreadyCreated = (new UserAccountsWrapper()).checkIfUserExist(userEmail);
        args.IsValid = !isUserAlreadyCreated;

    }

    [WebMethod]
    public static void UpdateCoordinates(Double la, Double lo)
    {
        
        latitude = la;
        longitude = lo;
       
    }
}