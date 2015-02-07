using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

/// <summary>
/// Summary description for SubscriptionWrapper
/// </summary>
public class SubscriptionWrapper : Suscription
{
    public void Save()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        Suscription sub = new Suscription();

        sub.RequestID = this.RequestID;
        sub.UserID = this.UserID;

        dc.Suscriptions.InsertOnSubmit(sub);
        dc.SubmitChanges();
    }

    public void update(Suscription sub)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        Suscription subAcc = dc.Suscriptions.Single(u => u.ID == sub.ID);

        subAcc.RequestID = sub.RequestID;
        subAcc.UserID = sub.UserID;

        dc.SubmitChanges();
    }

    public static Suscription getById(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        var sub = dc.Suscriptions.Single(u => u.ID == id);
        return sub;
    }

    public static List<Suscription> getByRequestId(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Suscription> enume = from p in dc.Suscriptions
                                         where p.RequestID == id
                                         select p;

        return enume.ToList();
    }

    public static List<Suscription> getByUserId(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Suscription> enume = from p in dc.Suscriptions 
                                         where p.UserID == id 
                                         select p;

        return enume.ToList();
    }

    public static List<Suscription> getAll()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Suscription> enume = from p in dc.Suscriptions select p;

        return enume.ToList();
    }
}