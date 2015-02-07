using Microsoft.Owin;
using Owin;

[assembly: OwinStartupAttribute(typeof(Sharetheload.Startup))]
namespace Sharetheload
{
    public partial class Startup {
        public void Configuration(IAppBuilder app) {
            ConfigureAuth(app);
        }
    }
}
