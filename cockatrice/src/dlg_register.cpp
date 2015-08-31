#include <QSettings>
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
    QSettings settings;
    settings.beginGroup("server");

    hostLabel = new QLabel(tr("&Host:"));
    hostEdit = new QLineEdit(settings.value("hostname", "cockatrice.woogerworks.com").toString());
    hostLabel->setBuddy(hostEdit);

    portLabel = new QLabel(tr("&Port:"));
    portEdit = new QLineEdit(settings.value("port", "4747").toString());
    portLabel->setBuddy(portEdit);

    playernameLabel = new QLabel(tr("Player &name:"));
    playernameEdit = new QLineEdit(settings.value("playername", "Player").toString());
    playernameLabel->setBuddy(playernameEdit);

    passwordLabel = new QLabel(tr("P&assword:"));
    passwordEdit = new QLineEdit(settings.value("password").toString());
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

    genderLabel = new QLabel(tr("Pronouns:"));
    genderEdit = new QComboBox();
    genderLabel->setBuddy(genderEdit);
    genderEdit->insertItem(0, QIcon("theme:genders/unknown.svg"), tr("Neutral"));
    genderEdit->insertItem(1, QIcon("theme:genders/male.svg"), tr("Masculine"));
    genderEdit->insertItem(2, QIcon("theme:genders/female.svg"), tr("Feminine"));
    genderEdit->setCurrentIndex(0);

    countryLabel = new QLabel(tr("Country:"));
    countryEdit = new QComboBox();
    countryLabel->setBuddy(countryEdit);
    countryEdit->insertItem(0, tr("Undefined"));
    countryEdit->addItem(QPixmap("theme:countries/ad.svg"), "ad");
    countryEdit->addItem(QIcon("theme:countries/ae.svg"), "ae");
    countryEdit->addItem(QIcon("theme:countries/af.svg"), "af");
    countryEdit->addItem(QIcon("theme:countries/ag.svg"), "ag");
    countryEdit->addItem(QIcon("theme:countries/ai.svg"), "ai");
    countryEdit->addItem(QIcon("theme:countries/al.svg"), "al");
    countryEdit->addItem(QIcon("theme:countries/am.svg"), "am");
    countryEdit->addItem(QIcon("theme:countries/ao.svg"), "ao");
    countryEdit->addItem(QIcon("theme:countries/aq.svg"), "aq");
    countryEdit->addItem(QIcon("theme:countries/ar.svg"), "ar");
    countryEdit->addItem(QIcon("theme:countries/as.svg"), "as");
    countryEdit->addItem(QIcon("theme:countries/at.svg"), "at");
    countryEdit->addItem(QIcon("theme:countries/au.svg"), "au");
    countryEdit->addItem(QIcon("theme:countries/aw.svg"), "aw");
    countryEdit->addItem(QIcon("theme:countries/ax.svg"), "ax");
    countryEdit->addItem(QIcon("theme:countries/az.svg"), "az");
    countryEdit->addItem(QIcon("theme:countries/ba.svg"), "ba");
    countryEdit->addItem(QIcon("theme:countries/bb.svg"), "bb");
    countryEdit->addItem(QIcon("theme:countries/bd.svg"), "bd");
    countryEdit->addItem(QIcon("theme:countries/be.svg"), "be");
    countryEdit->addItem(QIcon("theme:countries/bf.svg"), "bf");
    countryEdit->addItem(QIcon("theme:countries/bg.svg"), "bg");
    countryEdit->addItem(QIcon("theme:countries/bh.svg"), "bh");
    countryEdit->addItem(QIcon("theme:countries/bi.svg"), "bi");
    countryEdit->addItem(QIcon("theme:countries/bj.svg"), "bj");
    countryEdit->addItem(QIcon("theme:countries/bl.svg"), "bl");
    countryEdit->addItem(QIcon("theme:countries/bm.svg"), "bm");
    countryEdit->addItem(QIcon("theme:countries/bn.svg"), "bn");
    countryEdit->addItem(QIcon("theme:countries/bo.svg"), "bo");
    countryEdit->addItem(QIcon("theme:countries/bq.svg"), "bq");
    countryEdit->addItem(QIcon("theme:countries/br.svg"), "br");
    countryEdit->addItem(QIcon("theme:countries/bs.svg"), "bs");
    countryEdit->addItem(QIcon("theme:countries/bt.svg"), "bt");
    countryEdit->addItem(QIcon("theme:countries/bv.svg"), "bv");
    countryEdit->addItem(QIcon("theme:countries/bw.svg"), "bw");
    countryEdit->addItem(QIcon("theme:countries/by.svg"), "by");
    countryEdit->addItem(QIcon("theme:countries/bz.svg"), "bz");
    countryEdit->addItem(QIcon("theme:countries/ca.svg"), "ca");
    countryEdit->addItem(QIcon("theme:countries/cc.svg"), "cc");
    countryEdit->addItem(QIcon("theme:countries/cd.svg"), "cd");
    countryEdit->addItem(QIcon("theme:countries/cf.svg"), "cf");
    countryEdit->addItem(QIcon("theme:countries/cg.svg"), "cg");
    countryEdit->addItem(QIcon("theme:countries/ch.svg"), "ch");
    countryEdit->addItem(QIcon("theme:countries/ci.svg"), "ci");
    countryEdit->addItem(QIcon("theme:countries/ck.svg"), "ck");
    countryEdit->addItem(QIcon("theme:countries/cl.svg"), "cl");
    countryEdit->addItem(QIcon("theme:countries/cm.svg"), "cm");
    countryEdit->addItem(QIcon("theme:countries/cn.svg"), "cn");
    countryEdit->addItem(QIcon("theme:countries/co.svg"), "co");
    countryEdit->addItem(QIcon("theme:countries/cr.svg"), "cr");
    countryEdit->addItem(QIcon("theme:countries/cu.svg"), "cu");
    countryEdit->addItem(QIcon("theme:countries/cv.svg"), "cv");
    countryEdit->addItem(QIcon("theme:countries/cw.svg"), "cw");
    countryEdit->addItem(QIcon("theme:countries/cx.svg"), "cx");
    countryEdit->addItem(QIcon("theme:countries/cy.svg"), "cy");
    countryEdit->addItem(QIcon("theme:countries/cz.svg"), "cz");
    countryEdit->addItem(QIcon("theme:countries/de.svg"), "de");
    countryEdit->addItem(QIcon("theme:countries/dj.svg"), "dj");
    countryEdit->addItem(QIcon("theme:countries/dk.svg"), "dk");
    countryEdit->addItem(QIcon("theme:countries/dm.svg"), "dm");
    countryEdit->addItem(QIcon("theme:countries/do.svg"), "do");
    countryEdit->addItem(QIcon("theme:countries/dz.svg"), "dz");
    countryEdit->addItem(QIcon("theme:countries/ec.svg"), "ec");
    countryEdit->addItem(QIcon("theme:countries/ee.svg"), "ee");
    countryEdit->addItem(QIcon("theme:countries/eg.svg"), "eg");
    countryEdit->addItem(QIcon("theme:countries/eh.svg"), "eh");
    countryEdit->addItem(QIcon("theme:countries/er.svg"), "er");
    countryEdit->addItem(QIcon("theme:countries/es.svg"), "es");
    countryEdit->addItem(QIcon("theme:countries/et.svg"), "et");
    countryEdit->addItem(QIcon("theme:countries/fi.svg"), "fi");
    countryEdit->addItem(QIcon("theme:countries/fj.svg"), "fj");
    countryEdit->addItem(QIcon("theme:countries/fk.svg"), "fk");
    countryEdit->addItem(QIcon("theme:countries/fm.svg"), "fm");
    countryEdit->addItem(QIcon("theme:countries/fo.svg"), "fo");
    countryEdit->addItem(QIcon("theme:countries/fr.svg"), "fr");
    countryEdit->addItem(QIcon("theme:countries/ga.svg"), "ga");
    countryEdit->addItem(QIcon("theme:countries/gb.svg"), "gb");
    countryEdit->addItem(QIcon("theme:countries/gd.svg"), "gd");
    countryEdit->addItem(QIcon("theme:countries/ge.svg"), "ge");
    countryEdit->addItem(QIcon("theme:countries/gf.svg"), "gf");
    countryEdit->addItem(QIcon("theme:countries/gg.svg"), "gg");
    countryEdit->addItem(QIcon("theme:countries/gh.svg"), "gh");
    countryEdit->addItem(QIcon("theme:countries/gi.svg"), "gi");
    countryEdit->addItem(QIcon("theme:countries/gl.svg"), "gl");
    countryEdit->addItem(QIcon("theme:countries/gm.svg"), "gm");
    countryEdit->addItem(QIcon("theme:countries/gn.svg"), "gn");
    countryEdit->addItem(QIcon("theme:countries/gp.svg"), "gp");
    countryEdit->addItem(QIcon("theme:countries/gq.svg"), "gq");
    countryEdit->addItem(QIcon("theme:countries/gr.svg"), "gr");
    countryEdit->addItem(QIcon("theme:countries/gs.svg"), "gs");
    countryEdit->addItem(QIcon("theme:countries/gt.svg"), "gt");
    countryEdit->addItem(QIcon("theme:countries/gu.svg"), "gu");
    countryEdit->addItem(QIcon("theme:countries/gw.svg"), "gw");
    countryEdit->addItem(QIcon("theme:countries/gy.svg"), "gy");
    countryEdit->addItem(QIcon("theme:countries/hk.svg"), "hk");
    countryEdit->addItem(QIcon("theme:countries/hm.svg"), "hm");
    countryEdit->addItem(QIcon("theme:countries/hn.svg"), "hn");
    countryEdit->addItem(QIcon("theme:countries/hr.svg"), "hr");
    countryEdit->addItem(QIcon("theme:countries/ht.svg"), "ht");
    countryEdit->addItem(QIcon("theme:countries/hu.svg"), "hu");
    countryEdit->addItem(QIcon("theme:countries/id.svg"), "id");
    countryEdit->addItem(QIcon("theme:countries/ie.svg"), "ie");
    countryEdit->addItem(QIcon("theme:countries/il.svg"), "il");
    countryEdit->addItem(QIcon("theme:countries/im.svg"), "im");
    countryEdit->addItem(QIcon("theme:countries/in.svg"), "in");
    countryEdit->addItem(QIcon("theme:countries/io.svg"), "io");
    countryEdit->addItem(QIcon("theme:countries/iq.svg"), "iq");
    countryEdit->addItem(QIcon("theme:countries/ir.svg"), "ir");
    countryEdit->addItem(QIcon("theme:countries/is.svg"), "is");
    countryEdit->addItem(QIcon("theme:countries/it.svg"), "it");
    countryEdit->addItem(QIcon("theme:countries/je.svg"), "je");
    countryEdit->addItem(QIcon("theme:countries/jm.svg"), "jm");
    countryEdit->addItem(QIcon("theme:countries/jo.svg"), "jo");
    countryEdit->addItem(QIcon("theme:countries/jp.svg"), "jp");
    countryEdit->addItem(QIcon("theme:countries/ke.svg"), "ke");
    countryEdit->addItem(QIcon("theme:countries/kg.svg"), "kg");
    countryEdit->addItem(QIcon("theme:countries/kh.svg"), "kh");
    countryEdit->addItem(QIcon("theme:countries/ki.svg"), "ki");
    countryEdit->addItem(QIcon("theme:countries/km.svg"), "km");
    countryEdit->addItem(QIcon("theme:countries/kn.svg"), "kn");
    countryEdit->addItem(QIcon("theme:countries/kp.svg"), "kp");
    countryEdit->addItem(QIcon("theme:countries/kr.svg"), "kr");
    countryEdit->addItem(QIcon("theme:countries/kw.svg"), "kw");
    countryEdit->addItem(QIcon("theme:countries/ky.svg"), "ky");
    countryEdit->addItem(QIcon("theme:countries/kz.svg"), "kz");
    countryEdit->addItem(QIcon("theme:countries/la.svg"), "la");
    countryEdit->addItem(QIcon("theme:countries/lb.svg"), "lb");
    countryEdit->addItem(QIcon("theme:countries/lc.svg"), "lc");
    countryEdit->addItem(QIcon("theme:countries/li.svg"), "li");
    countryEdit->addItem(QIcon("theme:countries/lk.svg"), "lk");
    countryEdit->addItem(QIcon("theme:countries/lr.svg"), "lr");
    countryEdit->addItem(QIcon("theme:countries/ls.svg"), "ls");
    countryEdit->addItem(QIcon("theme:countries/lt.svg"), "lt");
    countryEdit->addItem(QIcon("theme:countries/lu.svg"), "lu");
    countryEdit->addItem(QIcon("theme:countries/lv.svg"), "lv");
    countryEdit->addItem(QIcon("theme:countries/ly.svg"), "ly");
    countryEdit->addItem(QIcon("theme:countries/ma.svg"), "ma");
    countryEdit->addItem(QIcon("theme:countries/mc.svg"), "mc");
    countryEdit->addItem(QIcon("theme:countries/md.svg"), "md");
    countryEdit->addItem(QIcon("theme:countries/me.svg"), "me");
    countryEdit->addItem(QIcon("theme:countries/mf.svg"), "mf");
    countryEdit->addItem(QIcon("theme:countries/mg.svg"), "mg");
    countryEdit->addItem(QIcon("theme:countries/mh.svg"), "mh");
    countryEdit->addItem(QIcon("theme:countries/mk.svg"), "mk");
    countryEdit->addItem(QIcon("theme:countries/ml.svg"), "ml");
    countryEdit->addItem(QIcon("theme:countries/mm.svg"), "mm");
    countryEdit->addItem(QIcon("theme:countries/mn.svg"), "mn");
    countryEdit->addItem(QIcon("theme:countries/mo.svg"), "mo");
    countryEdit->addItem(QIcon("theme:countries/mp.svg"), "mp");
    countryEdit->addItem(QIcon("theme:countries/mq.svg"), "mq");
    countryEdit->addItem(QIcon("theme:countries/mr.svg"), "mr");
    countryEdit->addItem(QIcon("theme:countries/ms.svg"), "ms");
    countryEdit->addItem(QIcon("theme:countries/mt.svg"), "mt");
    countryEdit->addItem(QIcon("theme:countries/mu.svg"), "mu");
    countryEdit->addItem(QIcon("theme:countries/mv.svg"), "mv");
    countryEdit->addItem(QIcon("theme:countries/mw.svg"), "mw");
    countryEdit->addItem(QIcon("theme:countries/mx.svg"), "mx");
    countryEdit->addItem(QIcon("theme:countries/my.svg"), "my");
    countryEdit->addItem(QIcon("theme:countries/mz.svg"), "mz");
    countryEdit->addItem(QIcon("theme:countries/na.svg"), "na");
    countryEdit->addItem(QIcon("theme:countries/nc.svg"), "nc");
    countryEdit->addItem(QIcon("theme:countries/ne.svg"), "ne");
    countryEdit->addItem(QIcon("theme:countries/nf.svg"), "nf");
    countryEdit->addItem(QIcon("theme:countries/ng.svg"), "ng");
    countryEdit->addItem(QIcon("theme:countries/ni.svg"), "ni");
    countryEdit->addItem(QIcon("theme:countries/nl.svg"), "nl");
    countryEdit->addItem(QIcon("theme:countries/no.svg"), "no");
    countryEdit->addItem(QIcon("theme:countries/np.svg"), "np");
    countryEdit->addItem(QIcon("theme:countries/nr.svg"), "nr");
    countryEdit->addItem(QIcon("theme:countries/nu.svg"), "nu");
    countryEdit->addItem(QIcon("theme:countries/nz.svg"), "nz");
    countryEdit->addItem(QIcon("theme:countries/om.svg"), "om");
    countryEdit->addItem(QIcon("theme:countries/pa.svg"), "pa");
    countryEdit->addItem(QIcon("theme:countries/pe.svg"), "pe");
    countryEdit->addItem(QIcon("theme:countries/pf.svg"), "pf");
    countryEdit->addItem(QIcon("theme:countries/pg.svg"), "pg");
    countryEdit->addItem(QIcon("theme:countries/ph.svg"), "ph");
    countryEdit->addItem(QIcon("theme:countries/pk.svg"), "pk");
    countryEdit->addItem(QIcon("theme:countries/pl.svg"), "pl");
    countryEdit->addItem(QIcon("theme:countries/pm.svg"), "pm");
    countryEdit->addItem(QIcon("theme:countries/pn.svg"), "pn");
    countryEdit->addItem(QIcon("theme:countries/pr.svg"), "pr");
    countryEdit->addItem(QIcon("theme:countries/ps.svg"), "ps");
    countryEdit->addItem(QIcon("theme:countries/pt.svg"), "pt");
    countryEdit->addItem(QIcon("theme:countries/pw.svg"), "pw");
    countryEdit->addItem(QIcon("theme:countries/py.svg"), "py");
    countryEdit->addItem(QIcon("theme:countries/qa.svg"), "qa");
    countryEdit->addItem(QIcon("theme:countries/re.svg"), "re");
    countryEdit->addItem(QIcon("theme:countries/ro.svg"), "ro");
    countryEdit->addItem(QIcon("theme:countries/rs.svg"), "rs");
    countryEdit->addItem(QIcon("theme:countries/ru.svg"), "ru");
    countryEdit->addItem(QIcon("theme:countries/rw.svg"), "rw");
    countryEdit->addItem(QIcon("theme:countries/sa.svg"), "sa");
    countryEdit->addItem(QIcon("theme:countries/sb.svg"), "sb");
    countryEdit->addItem(QIcon("theme:countries/sc.svg"), "sc");
    countryEdit->addItem(QIcon("theme:countries/sd.svg"), "sd");
    countryEdit->addItem(QIcon("theme:countries/se.svg"), "se");
    countryEdit->addItem(QIcon("theme:countries/sg.svg"), "sg");
    countryEdit->addItem(QIcon("theme:countries/sh.svg"), "sh");
    countryEdit->addItem(QIcon("theme:countries/si.svg"), "si");
    countryEdit->addItem(QIcon("theme:countries/sj.svg"), "sj");
    countryEdit->addItem(QIcon("theme:countries/sk.svg"), "sk");
    countryEdit->addItem(QIcon("theme:countries/sl.svg"), "sl");
    countryEdit->addItem(QIcon("theme:countries/sm.svg"), "sm");
    countryEdit->addItem(QIcon("theme:countries/sn.svg"), "sn");
    countryEdit->addItem(QIcon("theme:countries/so.svg"), "so");
    countryEdit->addItem(QIcon("theme:countries/sr.svg"), "sr");
    countryEdit->addItem(QIcon("theme:countries/ss.svg"), "ss");
    countryEdit->addItem(QIcon("theme:countries/st.svg"), "st");
    countryEdit->addItem(QIcon("theme:countries/sv.svg"), "sv");
    countryEdit->addItem(QIcon("theme:countries/sx.svg"), "sx");
    countryEdit->addItem(QIcon("theme:countries/sy.svg"), "sy");
    countryEdit->addItem(QIcon("theme:countries/sz.svg"), "sz");
    countryEdit->addItem(QIcon("theme:countries/tc.svg"), "tc");
    countryEdit->addItem(QIcon("theme:countries/td.svg"), "td");
    countryEdit->addItem(QIcon("theme:countries/tf.svg"), "tf");
    countryEdit->addItem(QIcon("theme:countries/tg.svg"), "tg");
    countryEdit->addItem(QIcon("theme:countries/th.svg"), "th");
    countryEdit->addItem(QIcon("theme:countries/tj.svg"), "tj");
    countryEdit->addItem(QIcon("theme:countries/tk.svg"), "tk");
    countryEdit->addItem(QIcon("theme:countries/tl.svg"), "tl");
    countryEdit->addItem(QIcon("theme:countries/tm.svg"), "tm");
    countryEdit->addItem(QIcon("theme:countries/tn.svg"), "tn");
    countryEdit->addItem(QIcon("theme:countries/to.svg"), "to");
    countryEdit->addItem(QIcon("theme:countries/tr.svg"), "tr");
    countryEdit->addItem(QIcon("theme:countries/tt.svg"), "tt");
    countryEdit->addItem(QIcon("theme:countries/tv.svg"), "tv");
    countryEdit->addItem(QIcon("theme:countries/tw.svg"), "tw");
    countryEdit->addItem(QIcon("theme:countries/tz.svg"), "tz");
    countryEdit->addItem(QIcon("theme:countries/ua.svg"), "ua");
    countryEdit->addItem(QIcon("theme:countries/ug.svg"), "ug");
    countryEdit->addItem(QIcon("theme:countries/um.svg"), "um");
    countryEdit->addItem(QIcon("theme:countries/us.svg"), "us");
    countryEdit->addItem(QIcon("theme:countries/uy.svg"), "uy");
    countryEdit->addItem(QIcon("theme:countries/uz.svg"), "uz");
    countryEdit->addItem(QIcon("theme:countries/va.svg"), "va");
    countryEdit->addItem(QIcon("theme:countries/vc.svg"), "vc");
    countryEdit->addItem(QIcon("theme:countries/ve.svg"), "ve");
    countryEdit->addItem(QIcon("theme:countries/vg.svg"), "vg");
    countryEdit->addItem(QIcon("theme:countries/vi.svg"), "vi");
    countryEdit->addItem(QIcon("theme:countries/vn.svg"), "vn");
    countryEdit->addItem(QIcon("theme:countries/vu.svg"), "vu");
    countryEdit->addItem(QIcon("theme:countries/wf.svg"), "wf");
    countryEdit->addItem(QIcon("theme:countries/ws.svg"), "ws");
    countryEdit->addItem(QIcon("theme:countries/ye.svg"), "ye");
    countryEdit->addItem(QIcon("theme:countries/yt.svg"), "yt");
    countryEdit->addItem(QIcon("theme:countries/za.svg"), "za");
    countryEdit->addItem(QIcon("theme:countries/zm.svg"), "zm");
    countryEdit->addItem(QIcon("theme:countries/zw.svg"), "zw");
    countryEdit->setCurrentIndex(0);
    QStringList countries = settingsCache->getCountries();
    foreach(QString c, countries)
        countryEdit->addItem(QPixmap(":/resources/countries/" + c + ".svg"), c);

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
    grid->addWidget(genderLabel, 7, 0);
    grid->addWidget(genderEdit, 7, 1);
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

    QSettings settings;
    settings.beginGroup("server");
    settings.setValue("hostname", hostEdit->text());
    settings.setValue("port", portEdit->text());
    settings.setValue("playername", playernameEdit->text());
    // always save the password so it will be picked up by the connect dialog
    settings.setValue("password", passwordEdit->text());
    settings.endGroup();
  
    accept();
}

void DlgRegister::actCancel()
{
    reject();
}
