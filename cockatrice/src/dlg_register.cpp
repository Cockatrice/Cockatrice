#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

#include "dlg_register.h"
#include "settingscache.h"
#include "pb/serverinfo_user.pb.h"

DlgRegister::DlgRegister(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settingsCache->servers().getHostname("cockatrice.woogerworks.com"));
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settingsCache->servers().getPort("4747"));
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settingsCache->servers().getPlayerName("Player"));
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settingsCache->servers().getPassword());
    passwordLabel->setBuddy(passwordEdit);
    passwordEdit->setEchoMode(QLineEdit::Password);

    passwordConfirmationLabel = new QLabel(tr("Password (again):"));
    passwordConfirmationEdit = new QLineEdit();
    passwordConfirmationLabel->setBuddy(passwordConfirmationEdit);
    passwordConfirmationEdit->setEchoMode(QLineEdit::Password);

    emailLabel = new QLabel(tr("Email:"));
    emailEdit = new QLineEdit();
    emailLabel->setBuddy(emailEdit);

    emailConfirmationLabel = new QLabel(tr("Email (again):"));
    emailConfirmationEdit = new QLineEdit();
    emailConfirmationLabel->setBuddy(emailConfirmationEdit);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->addItem(QPixmap("theme:countries/ad"), "ad");
    countryEdit->addItem(QPixmap("theme:countries/ae"), "ae");
    countryEdit->addItem(QPixmap("theme:countries/af"), "af");
    countryEdit->addItem(QPixmap("theme:countries/ag"), "ag");
    countryEdit->addItem(QPixmap("theme:countries/ai"), "ai");
    countryEdit->addItem(QPixmap("theme:countries/al"), "al");
    countryEdit->addItem(QPixmap("theme:countries/am"), "am");
    countryEdit->addItem(QPixmap("theme:countries/ao"), "ao");
    countryEdit->addItem(QPixmap("theme:countries/aq"), "aq");
    countryEdit->addItem(QPixmap("theme:countries/ar"), "ar");
    countryEdit->addItem(QPixmap("theme:countries/as"), "as");
    countryEdit->addItem(QPixmap("theme:countries/at"), "at");
    countryEdit->addItem(QPixmap("theme:countries/au"), "au");
    countryEdit->addItem(QPixmap("theme:countries/aw"), "aw");
    countryEdit->addItem(QPixmap("theme:countries/ax"), "ax");
    countryEdit->addItem(QPixmap("theme:countries/az"), "az");
    countryEdit->addItem(QPixmap("theme:countries/ba"), "ba");
    countryEdit->addItem(QPixmap("theme:countries/bb"), "bb");
    countryEdit->addItem(QPixmap("theme:countries/bd"), "bd");
    countryEdit->addItem(QPixmap("theme:countries/be"), "be");
    countryEdit->addItem(QPixmap("theme:countries/bf"), "bf");
    countryEdit->addItem(QPixmap("theme:countries/bg"), "bg");
    countryEdit->addItem(QPixmap("theme:countries/bh"), "bh");
    countryEdit->addItem(QPixmap("theme:countries/bi"), "bi");
    countryEdit->addItem(QPixmap("theme:countries/bj"), "bj");
    countryEdit->addItem(QPixmap("theme:countries/bl"), "bl");
    countryEdit->addItem(QPixmap("theme:countries/bm"), "bm");
    countryEdit->addItem(QPixmap("theme:countries/bn"), "bn");
    countryEdit->addItem(QPixmap("theme:countries/bo"), "bo");
    countryEdit->addItem(QPixmap("theme:countries/bq"), "bq");
    countryEdit->addItem(QPixmap("theme:countries/br"), "br");
    countryEdit->addItem(QPixmap("theme:countries/bs"), "bs");
    countryEdit->addItem(QPixmap("theme:countries/bt"), "bt");
    countryEdit->addItem(QPixmap("theme:countries/bv"), "bv");
    countryEdit->addItem(QPixmap("theme:countries/bw"), "bw");
    countryEdit->addItem(QPixmap("theme:countries/by"), "by");
    countryEdit->addItem(QPixmap("theme:countries/bz"), "bz");
    countryEdit->addItem(QPixmap("theme:countries/ca"), "ca");
    countryEdit->addItem(QPixmap("theme:countries/cc"), "cc");
    countryEdit->addItem(QPixmap("theme:countries/cd"), "cd");
    countryEdit->addItem(QPixmap("theme:countries/cf"), "cf");
    countryEdit->addItem(QPixmap("theme:countries/cg"), "cg");
    countryEdit->addItem(QPixmap("theme:countries/ch"), "ch");
    countryEdit->addItem(QPixmap("theme:countries/ci"), "ci");
    countryEdit->addItem(QPixmap("theme:countries/ck"), "ck");
    countryEdit->addItem(QPixmap("theme:countries/cl"), "cl");
    countryEdit->addItem(QPixmap("theme:countries/cm"), "cm");
    countryEdit->addItem(QPixmap("theme:countries/cn"), "cn");
    countryEdit->addItem(QPixmap("theme:countries/co"), "co");
    countryEdit->addItem(QPixmap("theme:countries/cr"), "cr");
    countryEdit->addItem(QPixmap("theme:countries/cu"), "cu");
    countryEdit->addItem(QPixmap("theme:countries/cv"), "cv");
    countryEdit->addItem(QPixmap("theme:countries/cw"), "cw");
    countryEdit->addItem(QPixmap("theme:countries/cx"), "cx");
    countryEdit->addItem(QPixmap("theme:countries/cy"), "cy");
    countryEdit->addItem(QPixmap("theme:countries/cz"), "cz");
    countryEdit->addItem(QPixmap("theme:countries/de"), "de");
    countryEdit->addItem(QPixmap("theme:countries/dj"), "dj");
    countryEdit->addItem(QPixmap("theme:countries/dk"), "dk");
    countryEdit->addItem(QPixmap("theme:countries/dm"), "dm");
    countryEdit->addItem(QPixmap("theme:countries/do"), "do");
    countryEdit->addItem(QPixmap("theme:countries/dz"), "dz");
    countryEdit->addItem(QPixmap("theme:countries/ec"), "ec");
    countryEdit->addItem(QPixmap("theme:countries/ee"), "ee");
    countryEdit->addItem(QPixmap("theme:countries/eg"), "eg");
    countryEdit->addItem(QPixmap("theme:countries/eh"), "eh");
    countryEdit->addItem(QPixmap("theme:countries/er"), "er");
    countryEdit->addItem(QPixmap("theme:countries/es"), "es");
    countryEdit->addItem(QPixmap("theme:countries/et"), "et");
    countryEdit->addItem(QPixmap("theme:countries/fi"), "fi");
    countryEdit->addItem(QPixmap("theme:countries/fj"), "fj");
    countryEdit->addItem(QPixmap("theme:countries/fk"), "fk");
    countryEdit->addItem(QPixmap("theme:countries/fm"), "fm");
    countryEdit->addItem(QPixmap("theme:countries/fo"), "fo");
    countryEdit->addItem(QPixmap("theme:countries/fr"), "fr");
    countryEdit->addItem(QPixmap("theme:countries/ga"), "ga");
    countryEdit->addItem(QPixmap("theme:countries/gb"), "gb");
    countryEdit->addItem(QPixmap("theme:countries/gd"), "gd");
    countryEdit->addItem(QPixmap("theme:countries/ge"), "ge");
    countryEdit->addItem(QPixmap("theme:countries/gf"), "gf");
    countryEdit->addItem(QPixmap("theme:countries/gg"), "gg");
    countryEdit->addItem(QPixmap("theme:countries/gh"), "gh");
    countryEdit->addItem(QPixmap("theme:countries/gi"), "gi");
    countryEdit->addItem(QPixmap("theme:countries/gl"), "gl");
    countryEdit->addItem(QPixmap("theme:countries/gm"), "gm");
    countryEdit->addItem(QPixmap("theme:countries/gn"), "gn");
    countryEdit->addItem(QPixmap("theme:countries/gp"), "gp");
    countryEdit->addItem(QPixmap("theme:countries/gq"), "gq");
    countryEdit->addItem(QPixmap("theme:countries/gr"), "gr");
    countryEdit->addItem(QPixmap("theme:countries/gs"), "gs");
    countryEdit->addItem(QPixmap("theme:countries/gt"), "gt");
    countryEdit->addItem(QPixmap("theme:countries/gu"), "gu");
    countryEdit->addItem(QPixmap("theme:countries/gw"), "gw");
    countryEdit->addItem(QPixmap("theme:countries/gy"), "gy");
    countryEdit->addItem(QPixmap("theme:countries/hk"), "hk");
    countryEdit->addItem(QPixmap("theme:countries/hm"), "hm");
    countryEdit->addItem(QPixmap("theme:countries/hn"), "hn");
    countryEdit->addItem(QPixmap("theme:countries/hr"), "hr");
    countryEdit->addItem(QPixmap("theme:countries/ht"), "ht");
    countryEdit->addItem(QPixmap("theme:countries/hu"), "hu");
    countryEdit->addItem(QPixmap("theme:countries/id"), "id");
    countryEdit->addItem(QPixmap("theme:countries/ie"), "ie");
    countryEdit->addItem(QPixmap("theme:countries/il"), "il");
    countryEdit->addItem(QPixmap("theme:countries/im"), "im");
    countryEdit->addItem(QPixmap("theme:countries/in"), "in");
    countryEdit->addItem(QPixmap("theme:countries/io"), "io");
    countryEdit->addItem(QPixmap("theme:countries/iq"), "iq");
    countryEdit->addItem(QPixmap("theme:countries/ir"), "ir");
    countryEdit->addItem(QPixmap("theme:countries/is"), "is");
    countryEdit->addItem(QPixmap("theme:countries/it"), "it");
    countryEdit->addItem(QPixmap("theme:countries/je"), "je");
    countryEdit->addItem(QPixmap("theme:countries/jm"), "jm");
    countryEdit->addItem(QPixmap("theme:countries/jo"), "jo");
    countryEdit->addItem(QPixmap("theme:countries/jp"), "jp");
    countryEdit->addItem(QPixmap("theme:countries/ke"), "ke");
    countryEdit->addItem(QPixmap("theme:countries/kg"), "kg");
    countryEdit->addItem(QPixmap("theme:countries/kh"), "kh");
    countryEdit->addItem(QPixmap("theme:countries/ki"), "ki");
    countryEdit->addItem(QPixmap("theme:countries/km"), "km");
    countryEdit->addItem(QPixmap("theme:countries/kn"), "kn");
    countryEdit->addItem(QPixmap("theme:countries/kp"), "kp");
    countryEdit->addItem(QPixmap("theme:countries/kr"), "kr");
    countryEdit->addItem(QPixmap("theme:countries/kw"), "kw");
    countryEdit->addItem(QPixmap("theme:countries/ky"), "ky");
    countryEdit->addItem(QPixmap("theme:countries/kz"), "kz");
    countryEdit->addItem(QPixmap("theme:countries/la"), "la");
    countryEdit->addItem(QPixmap("theme:countries/lb"), "lb");
    countryEdit->addItem(QPixmap("theme:countries/lc"), "lc");
    countryEdit->addItem(QPixmap("theme:countries/li"), "li");
    countryEdit->addItem(QPixmap("theme:countries/lk"), "lk");
    countryEdit->addItem(QPixmap("theme:countries/lr"), "lr");
    countryEdit->addItem(QPixmap("theme:countries/ls"), "ls");
    countryEdit->addItem(QPixmap("theme:countries/lt"), "lt");
    countryEdit->addItem(QPixmap("theme:countries/lu"), "lu");
    countryEdit->addItem(QPixmap("theme:countries/lv"), "lv");
    countryEdit->addItem(QPixmap("theme:countries/ly"), "ly");
    countryEdit->addItem(QPixmap("theme:countries/ma"), "ma");
    countryEdit->addItem(QPixmap("theme:countries/mc"), "mc");
    countryEdit->addItem(QPixmap("theme:countries/md"), "md");
    countryEdit->addItem(QPixmap("theme:countries/me"), "me");
    countryEdit->addItem(QPixmap("theme:countries/mf"), "mf");
    countryEdit->addItem(QPixmap("theme:countries/mg"), "mg");
    countryEdit->addItem(QPixmap("theme:countries/mh"), "mh");
    countryEdit->addItem(QPixmap("theme:countries/mk"), "mk");
    countryEdit->addItem(QPixmap("theme:countries/ml"), "ml");
    countryEdit->addItem(QPixmap("theme:countries/mm"), "mm");
    countryEdit->addItem(QPixmap("theme:countries/mn"), "mn");
    countryEdit->addItem(QPixmap("theme:countries/mo"), "mo");
    countryEdit->addItem(QPixmap("theme:countries/mp"), "mp");
    countryEdit->addItem(QPixmap("theme:countries/mq"), "mq");
    countryEdit->addItem(QPixmap("theme:countries/mr"), "mr");
    countryEdit->addItem(QPixmap("theme:countries/ms"), "ms");
    countryEdit->addItem(QPixmap("theme:countries/mt"), "mt");
    countryEdit->addItem(QPixmap("theme:countries/mu"), "mu");
    countryEdit->addItem(QPixmap("theme:countries/mv"), "mv");
    countryEdit->addItem(QPixmap("theme:countries/mw"), "mw");
    countryEdit->addItem(QPixmap("theme:countries/mx"), "mx");
    countryEdit->addItem(QPixmap("theme:countries/my"), "my");
    countryEdit->addItem(QPixmap("theme:countries/mz"), "mz");
    countryEdit->addItem(QPixmap("theme:countries/na"), "na");
    countryEdit->addItem(QPixmap("theme:countries/nc"), "nc");
    countryEdit->addItem(QPixmap("theme:countries/ne"), "ne");
    countryEdit->addItem(QPixmap("theme:countries/nf"), "nf");
    countryEdit->addItem(QPixmap("theme:countries/ng"), "ng");
    countryEdit->addItem(QPixmap("theme:countries/ni"), "ni");
    countryEdit->addItem(QPixmap("theme:countries/nl"), "nl");
    countryEdit->addItem(QPixmap("theme:countries/no"), "no");
    countryEdit->addItem(QPixmap("theme:countries/np"), "np");
    countryEdit->addItem(QPixmap("theme:countries/nr"), "nr");
    countryEdit->addItem(QPixmap("theme:countries/nu"), "nu");
    countryEdit->addItem(QPixmap("theme:countries/nz"), "nz");
    countryEdit->addItem(QPixmap("theme:countries/om"), "om");
    countryEdit->addItem(QPixmap("theme:countries/pa"), "pa");
    countryEdit->addItem(QPixmap("theme:countries/pe"), "pe");
    countryEdit->addItem(QPixmap("theme:countries/pf"), "pf");
    countryEdit->addItem(QPixmap("theme:countries/pg"), "pg");
    countryEdit->addItem(QPixmap("theme:countries/ph"), "ph");
    countryEdit->addItem(QPixmap("theme:countries/pk"), "pk");
    countryEdit->addItem(QPixmap("theme:countries/pl"), "pl");
    countryEdit->addItem(QPixmap("theme:countries/pm"), "pm");
    countryEdit->addItem(QPixmap("theme:countries/pn"), "pn");
    countryEdit->addItem(QPixmap("theme:countries/pr"), "pr");
    countryEdit->addItem(QPixmap("theme:countries/ps"), "ps");
    countryEdit->addItem(QPixmap("theme:countries/pt"), "pt");
    countryEdit->addItem(QPixmap("theme:countries/pw"), "pw");
    countryEdit->addItem(QPixmap("theme:countries/py"), "py");
    countryEdit->addItem(QPixmap("theme:countries/qa"), "qa");
    countryEdit->addItem(QPixmap("theme:countries/re"), "re");
    countryEdit->addItem(QPixmap("theme:countries/ro"), "ro");
    countryEdit->addItem(QPixmap("theme:countries/rs"), "rs");
    countryEdit->addItem(QPixmap("theme:countries/ru"), "ru");
    countryEdit->addItem(QPixmap("theme:countries/rw"), "rw");
    countryEdit->addItem(QPixmap("theme:countries/sa"), "sa");
    countryEdit->addItem(QPixmap("theme:countries/sb"), "sb");
    countryEdit->addItem(QPixmap("theme:countries/sc"), "sc");
    countryEdit->addItem(QPixmap("theme:countries/sd"), "sd");
    countryEdit->addItem(QPixmap("theme:countries/se"), "se");
    countryEdit->addItem(QPixmap("theme:countries/sg"), "sg");
    countryEdit->addItem(QPixmap("theme:countries/sh"), "sh");
    countryEdit->addItem(QPixmap("theme:countries/si"), "si");
    countryEdit->addItem(QPixmap("theme:countries/sj"), "sj");
    countryEdit->addItem(QPixmap("theme:countries/sk"), "sk");
    countryEdit->addItem(QPixmap("theme:countries/sl"), "sl");
    countryEdit->addItem(QPixmap("theme:countries/sm"), "sm");
    countryEdit->addItem(QPixmap("theme:countries/sn"), "sn");
    countryEdit->addItem(QPixmap("theme:countries/so"), "so");
    countryEdit->addItem(QPixmap("theme:countries/sr"), "sr");
    countryEdit->addItem(QPixmap("theme:countries/ss"), "ss");
    countryEdit->addItem(QPixmap("theme:countries/st"), "st");
    countryEdit->addItem(QPixmap("theme:countries/sv"), "sv");
    countryEdit->addItem(QPixmap("theme:countries/sx"), "sx");
    countryEdit->addItem(QPixmap("theme:countries/sy"), "sy");
    countryEdit->addItem(QPixmap("theme:countries/sz"), "sz");
    countryEdit->addItem(QPixmap("theme:countries/tc"), "tc");
    countryEdit->addItem(QPixmap("theme:countries/td"), "td");
    countryEdit->addItem(QPixmap("theme:countries/tf"), "tf");
    countryEdit->addItem(QPixmap("theme:countries/tg"), "tg");
    countryEdit->addItem(QPixmap("theme:countries/th"), "th");
    countryEdit->addItem(QPixmap("theme:countries/tj"), "tj");
    countryEdit->addItem(QPixmap("theme:countries/tk"), "tk");
    countryEdit->addItem(QPixmap("theme:countries/tl"), "tl");
    countryEdit->addItem(QPixmap("theme:countries/tm"), "tm");
    countryEdit->addItem(QPixmap("theme:countries/tn"), "tn");
    countryEdit->addItem(QPixmap("theme:countries/to"), "to");
    countryEdit->addItem(QPixmap("theme:countries/tr"), "tr");
    countryEdit->addItem(QPixmap("theme:countries/tt"), "tt");
    countryEdit->addItem(QPixmap("theme:countries/tv"), "tv");
    countryEdit->addItem(QPixmap("theme:countries/tw"), "tw");
    countryEdit->addItem(QPixmap("theme:countries/tz"), "tz");
    countryEdit->addItem(QPixmap("theme:countries/ua"), "ua");
    countryEdit->addItem(QPixmap("theme:countries/ug"), "ug");
    countryEdit->addItem(QPixmap("theme:countries/um"), "um");
    countryEdit->addItem(QPixmap("theme:countries/us"), "us");
    countryEdit->addItem(QPixmap("theme:countries/uy"), "uy");
    countryEdit->addItem(QPixmap("theme:countries/uz"), "uz");
    countryEdit->addItem(QPixmap("theme:countries/va"), "va");
    countryEdit->addItem(QPixmap("theme:countries/vc"), "vc");
    countryEdit->addItem(QPixmap("theme:countries/ve"), "ve");
    countryEdit->addItem(QPixmap("theme:countries/vg"), "vg");
    countryEdit->addItem(QPixmap("theme:countries/vi"), "vi");
    countryEdit->addItem(QPixmap("theme:countries/vn"), "vn");
    countryEdit->addItem(QPixmap("theme:countries/vu"), "vu");
    countryEdit->addItem(QPixmap("theme:countries/wf"), "wf");
    countryEdit->addItem(QPixmap("theme:countries/ws"), "ws");
    countryEdit->addItem(QPixmap("theme:countries/ye"), "ye");
    countryEdit->addItem(QPixmap("theme:countries/yt"), "yt");
    countryEdit->addItem(QPixmap("theme:countries/za"), "za");
    countryEdit->addItem(QPixmap("theme:countries/zm"), "zm");
    countryEdit->addItem(QPixmap("theme:countries/zw"), "zw");
    countryEdit->setCurrentIndex(0);
    QStringList countries = settingsCache->getCountries();
    foreach(QString c, countries)
        countryEdit->addItem(QPixmap("theme:countries/" + c.toLower()), c);

    realnameLabel = new QLabel(tr("Real name:"));
    realnameEdit = new QLineEdit();
    realnameLabel->setBuddy(realnameEdit);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(hostLabel, 0, 0);
    grid->addWidget(hostEdit, 0, 1);
    grid->addWidget(portLabel, 1, 0);
    grid->addWidget(portEdit, 1, 1);
    grid->addWidget(playernameLabel, 2, 0);
    grid->addWidget(playernameEdit, 2, 1);
    grid->addWidget(passwordLabel, 3, 0);
    grid->addWidget(passwordEdit, 3, 1);
    grid->addWidget(passwordConfirmationLabel, 4, 0);
    grid->addWidget(passwordConfirmationEdit, 4, 1);
    grid->addWidget(emailLabel, 5, 0);
    grid->addWidget(emailEdit, 5, 1);
    grid->addWidget(emailConfirmationLabel, 6, 0);
    grid->addWidget(emailConfirmationEdit, 6, 1);
    grid->addWidget(countryLabel, 8, 0);
    grid->addWidget(countryEdit, 8, 1);
    grid->addWidget(realnameLabel, 9, 0);
    grid->addWidget(realnameEdit, 9, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(actCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Register to server"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgRegister::actOk()
{
    if (passwordEdit->text() != passwordConfirmationEdit->text())
    {
         QMessageBox::critical(this, tr("Registration Warning"), tr("Your passwords do not match, please try again."));
         return;
    }
    else if (emailConfirmationEdit->text() != emailEdit->text())
    {
        QMessageBox::critical(this, tr("Registration Warning"), tr("Your email addresses do not match, please try again."));
        return;
    }
    if(playernameEdit->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Registration Warning"), tr("The player name can't be empty."));
        return;
    }

    settingsCache->servers().setHostName(hostEdit->text());
    settingsCache->servers().setPort(portEdit->text());
    settingsCache->servers().setPlayerName(playernameEdit->text());
    // always save the password so it will be picked up by the connect dialog
    settingsCache->servers().setPassword(passwordEdit->text());

    accept();
}

void DlgRegister::actCancel()
{
    reject();
}
