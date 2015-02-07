namespace _1291
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    [Table("Request")]
    public partial class Request
    {
        public Request()
        {
            Suscription = new HashSet<Suscription>();
        }

        public int ID { get; set; }

        public int UserID { get; set; }

        public int ProductID { get; set; }

        [Required]
        public string Title { get; set; }

        [Column(TypeName = "date")]
        public DateTime StartDate { get; set; }

        [Column(TypeName = "date")]
        public DateTime EndDate { get; set; }

        public int MinUsers { get; set; }

        public int MaxUsers { get; set; }

        [Required]
        public string State { get; set; }

        [Required]
        public string City { get; set; }

        public string Sector { get; set; }

        [Required]
        public string Street { get; set; }

        [Required]
        public string Status { get; set; }

        public virtual Product Product { get; set; }

        public virtual UserAccount UserAccount { get; set; }

        public virtual ICollection<Suscription> Suscription { get; set; }
    }
}
