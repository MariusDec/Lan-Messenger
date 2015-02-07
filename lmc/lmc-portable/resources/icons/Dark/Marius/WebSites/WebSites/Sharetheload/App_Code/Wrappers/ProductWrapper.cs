using System;
using System.Collections.Generic;
using System.Data.Linq.SqlClient;
using System.Linq;
using System.Web;

/// <summary>
/// Summary description for ProductWrapper
/// </summary>
public class ProductWrapper:Product
{
    public void Save()
    {
        DataClassesDataContext dc = new DataClassesDataContext();

        Product prod = new Product();
        prod.Category = this.Category;
        prod.Description = this.Description;
        prod.Name = this.Name;
        prod.Price = this.Price;

        dc.Products.InsertOnSubmit(prod);
        dc.SubmitChanges();

        this.ID = prod.ID;
    }

    public static Product getById(int id)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        var product = dc.Products.Single(u => u.ID == id);
        return product;
    }

    public static List<Product> getAll()
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Product> enume = from p in dc.Products select p;

        return enume.ToList();
    }

    public static List<Product> getByName(String name)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Product> enume = from p in dc.Products where SqlMethods.Like(p.Name, "%" + name + "%") select p;

        return enume.ToList();
    }

    //get a description with search name in it
    public static List<Product> getByDescription(String name)
    {
        DataClassesDataContext dc = new DataClassesDataContext();
        IEnumerable<Product> enume = from p in dc.Products where SqlMethods.Like(p.Description, "%" + name + "%") select p;

        return enume.ToList();
    }
}