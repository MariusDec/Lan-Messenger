namespace _1291
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    [Table("UserAccount")]
    public partial class UserAccount
    {
        public UserAccount()
        {
            Request = new HashSet<Request>();
            Suscription = new HashSet<Suscription>();
        }

        [Key]
        public int UserID { get; set; }

        [Required]
        public string FirstName { get; set; }

        [Required]
        public string LastName { get; set; }

        [Required]
        public string Email { get; set; }

        [Required]
        public string Password { get; set; }

        [Required]
        [StringLength(15)]
        public string Phone { get; set; }

        public int AddressID { get; set; }

        public virtual Address Address { get; set; }

        public virtual ICollection<Request> Request { get; set; }

        public virtual ICollection<Suscription> Suscription { get; set; }
    }
}
