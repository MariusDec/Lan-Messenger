namespace _1291
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    [Table("Address")]
    public partial class Address
    {
        public Address()
        {
            UserAccount = new HashSet<UserAccount>();
        }

        public int ID { get; set; }

        [Required]
        public string Country { get; set; }

        [Required]
        public string State { get; set; }

        [Required]
        public string City { get; set; }

        public string Sector { get; set; }

        [Required]
        public string Street { get; set; }

        [Required]
        public string StreetNo { get; set; }

        [Required]
        public string Building { get; set; }

        [Required]
        public string Staircase { get; set; }

        [Required]
        public string Floor { get; set; }

        [Required]
        public string Apartment { get; set; }

        public virtual ICollection<UserAccount> UserAccount { get; set; }
    }
}
