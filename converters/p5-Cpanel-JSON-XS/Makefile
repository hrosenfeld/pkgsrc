# $NetBSD: Makefile,v 1.9 2019/08/11 13:18:12 wiz Exp $

DISTNAME=	Cpanel-JSON-XS-4.08
PKGNAME=	p5-${DISTNAME}
PKGREVISION=	1
CATEGORIES=	textproc perl5
MASTER_SITES=	${MASTER_SITE_PERL_CPAN:=../../authors/id/R/RU/RURBAN/}

MAINTAINER=	pkgsrc-users@NetBSD.org
HOMEPAGE=	https://metacpan.org/release/Cpanel-JSON-XS
COMMENT=	JSON serialising/deserialising, done correctly and fast (cPanel fork)
LICENSE=	${PERL5_LICENSE}

DEPENDS+=	p5-common-sense>=0:../../devel/p5-common-sense
DEPENDS+=	p5-Canary-Stability>=0:../../devel/p5-Canary-Stability
DEPENDS+=	p5-Types-Serialiser>=0:../../devel/p5-Types-Serialiser

PERL5_PACKLIST=	auto/Cpanel/JSON/XS/.packlist

.include "../../lang/perl5/module.mk"
.include "../../mk/bsd.pkg.mk"
