namespace _1291
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    [Table("Suscription")]
    public partial class Suscription
    {
        public int ID { get; set; }

        public int RequestID { get; set; }

        public int UserID { get; set; }

        public virtual Request Request { get; set; }

        public virtual UserAccount UserAccount { get; set; }
    }
}
