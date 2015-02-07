using System;
using System.Collections.Generic;
using System.Data.Linq.SqlClient;
using System.Globalization;
using System.Linq;
using System.Web;

/// <summary>
/// Summary description for RequestWrapper
/// </summary>
public class RequestWrapper : Request
{
    public void Save()
    {
        DataClassesDataContext dc = new DataClassesDataContext();

        Request req = new Request();
        req.UserID = this.UserID;
        req.ProductID = this.ProductID;
        req.Title = this.Title;
        req.StartDate = this.StartDate;
        req.EndDate = this.EndDate;
        req.MinUsers = this.MinUsers;
        req.MaxUsers = this.MaxUsers;
        req.Status = this.Status;
        req.Latitude = this.Latitude;
        req.Longitude = this.Longitude;
        req.Radius = this.Radius;

        dc.Requests.InsertOnSubmit(req);
        dc.SubmitChanges();

        this.ID = req.ID;
    }

    public void update(Request req)
    {
        DataClassesDataContext dc = new DataClassesDataContext();

        req.Title = this.Title;
        req.StartDate = this.StartDate;
        req.EndDate = this.EndDate;
        req.MinUsers = this.MinUsers;
        req.MaxUsers = this.MaxUsers;
        req.Status = this.Status;
        req.Latitude = this.Latitude;
        req.Longitude = this.Longitude;
        req.Radius = this.Radius;

        dc.SubmitChanges();
    }

    public static Request getById(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        var req = dc.Requests.Single(u => u.ID == id);
        return req;
    }

    public static List<Request> getAllRequests(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Request> enume = from p in dc.Requests 
                                         where p.UserID == id 
                                         select p;

        return enume.ToList();
    }

    public static List<Request> getAll()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        var products = from p in dc.Requests 
                       select p;
        /*List<Request> requests = new List<Request>();
        foreach (Request name in products)
        {
            requests.Add(name);   
        }
        return requests;*/
        return products.ToList();
    }

    public static List<Request> GetAllInRange(int userID, Double latitude, Double longitude)
    {
        List<Request> inRange = new List<Request>();

        DataClassesDataContext dc = new DataClassesDataContext();
        var products = from p in dc.Requests
                       where p.UserID != userID
                       select p;

        List<Request> allProducts = products.ToList();

        foreach(Request req in allProducts)
        {
            Double latit;
            Double longit;
            var style = NumberStyles.Number | NumberStyles.AllowCurrencySymbol;
            var culture = CultureInfo.CreateSpecificCulture("en-GB");
            Double.TryParse(req.Latitude, style, culture, out latit);
            Double.TryParse(req.Longitude, style, culture, out longit);
            double dist = Math.Sqrt((latit - latitude) * (latit - latitude) + (longit - longitude) * (longit - longitude));

            if (dist <= req.Radius)
            {
                inRange.Add(req);
            }
        }

        return inRange;
    }

    public static List<Request> getName(String name)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        var enume = from p in dc.Requests where SqlMethods.Like(p.Title, "%" + name + "%") select p;
        List<Request> allProducts = enume.ToList();
        return allProducts;
    }
}