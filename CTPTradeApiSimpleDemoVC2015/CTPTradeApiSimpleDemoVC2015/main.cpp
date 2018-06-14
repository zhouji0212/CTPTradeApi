/*******************************************************
* CTPTradeApiSimpleDemoVC2015
* www.xfinapi.com
*******************************************************/

/*
VC++2015��������

1������Ŀ¼
ѡ�񹤳�-����-VC++Ŀ¼-����Ŀ¼
���ӣ�$(Solutiondir)..\TradeApi_win32x86\Cpp

2��lib��Ŀ¼
ѡ�񹤳�-����-VC++Ŀ¼-��Ŀ¼
���ӣ�$(Solutiondir)..\TradeApi_win32x86\Cpp

3��lib������
ѡ�񹤳�-����-������-����-����������
���ӣ�
releaseʹ�ã�XFinApi.ITradeApi.lib
��
debugʹ�ã�XFinApi.ITradeApid.lib

4�����Թ���Ŀ¼
ѡ�񹤳�-����-����-����Ŀ¼
�޸�Ϊ��$(TargetDir)

5���������ļ�
�ο�copy.bat�ֶ�����
��
����ʱ�Զ�������
ѡ�񹤳�-����-�����¼�-���������¼�-�����У����ӣ�copy.bat
*/

/*
������ɵĳ��򷢲�

1.����XFinApi.ITradeApi.dll����ִ���ļ�Ŀ¼

2.����TradeApi_win32x86\Api\CTPTradeApi_vX.X.X_XXXXXXXXĿ¼��X.X.X_XXXXXXXXΪ�汾�ţ��µ�����dll�ļ�����ִ���ļ�Ŀ¼��
���鱣��dll�ļ�����Ŀ¼�ṹ������ӿ�dll�ļ��������������⡣
*/

#include <iostream>
#include <algorithm>
#include <thread>

#include "XFinApi.h"

//////////////////////////////////////////////////////////////////////////////////
//������Ϣ
class Config
{
public:
	//��ַ
	std::string MarketAddress;
	std::string TradeAddress;

	//�˻�
	std::string BrokerID;
	std::string UserName;
	std::string Password;

	//��Լ
	std::string InstrumentID;

	//����
	double SellPrice1 = -1;
	double BuyPrice1 = -1;

	Config()
	{
		//ע��CTP���潻���˺ţ�www.simnow.com.cn

		//����ʱ��
		MarketAddress = "180.168.146.187:10010";
		TradeAddress = "180.168.146.187:10000";

		BrokerID = "9999";
		UserName = "119486";
		Password = "a123456";

		InstrumentID = "au1812";
	}

	void SetNonTradeTime()
	{
		//�ǽ���ʱ��
		//�������̺󣬻�ż�ʱ��
		MarketAddress = "180.168.146.187:10031";
		TradeAddress = "180.168.146.187:10030";
	}
};

class MarketEvent;
class TradeEvent;

//////////////////////////////////////////////////////////////////////////////////
static Config Cfg;
static XFinApi::TradeApi::IMarket *market = nullptr;
static XFinApi::TradeApi::ITrade *trade = nullptr;
static MarketEvent *marketEvent = nullptr;
static TradeEvent *tradeEvent = nullptr;

//////////////////////////////////////////////////////////////////////////////////
//��������
#define DEFAULT_FILTER(_x)  ( XFinApi::TradeApi::IsDefaultValue(_x) ? -1 : _x)

static void PrintNotifyInfo(const XFinApi::TradeApi::NotifyParams &param)
{
	std::string strs;
	for (const XFinApi::TradeApi::CodeInfo &info : param.CodeInfos)
	{
		strs += "(Code=" + info.Code +
			";LowerCode=" + info.LowerCode +
			";LowerMessage=" + info.LowerMessage + ")";
	}
	printf(" OnNotify: Action=%d, Result=%d%s\n",
		param.Action,
		param.Result,
		strs.c_str());
}

static void PrintSubscribedInfo(const XFinApi::TradeApi::QueryParams &instInfo)
{
	printf("- OnSubscribed: %s\n", instInfo.InstrumentID.c_str());
}

static void PrintUnsubscribedInfo(const XFinApi::TradeApi::QueryParams &instInfo)
{
	printf("- OnUnsubscribed: %s\n", instInfo.InstrumentID.c_str());
}

static void PrintTickInfo(const XFinApi::TradeApi::Tick &tick)
{
	printf("  Tick,%s %s, HighestPrice=%g, LowestPrice=%g, BidPrice0=%g, BidVolume0=%lld, AskPrice0=%g, AskVolume0=%lld, LastPrice=%g, LastVolume=%lld, TradingDay=%s, TradingTime=%s\n",
		tick.ExchangeID.c_str(),
		tick.InstrumentID.c_str(),
		tick.HighestPrice,
		tick.LowestPrice,
		tick.BidPrice[0],
		tick.BidVolume[0],
		tick.AskPrice[0],
		tick.AskVolume[0],
		tick.LastPrice,
		tick.LastVolume,
		tick.TradingDay.c_str(),
		tick.TradingTime.c_str());
}

static void  PrintOrderInfo(const XFinApi::TradeApi::Order &order)
{
	printf("  ProductType=%d, Ref=%s, ID=%s, InstID=%s, Price=%g, Volume=%lld, NoTradedVolume=%lld, Direction=%d, OpenCloseType=%d, PriceCond=%d, TimeCond=%d, VolumeCond=%d, Status=%d, Msg=%s, %s, FrontID=%lld, SessionID=%lld\n",
		order.ProductType,
		order.OrderRef.c_str(), order.OrderID.c_str(),
		order.InstrumentID.c_str(), order.Price, order.Volume, order.NoTradedVolume,
		order.Direction, order.OpenCloseType,
		order.PriceCond,
		order.TimeCond,
		order.VolumeCond,
		order.Status,
		order.StatusMsg.c_str(),
		order.OrderTime.c_str(),
		order.FrontID,
		order.SessionID
	);
}

static void  PrintTradeInfo(const XFinApi::TradeApi::TradeOrder &trade)
{
	printf("  ID=%s, OrderID=%s, InstID=%s, Price=%g, Volume=%lld, Direction=%d, OpenCloseType=%d, %s\n",
		trade.TradeID.c_str(), trade.OrderID.c_str(),
		trade.InstrumentID.c_str(), trade.Price, trade.Volume,
		trade.Direction, trade.OpenCloseType,
		trade.TradeTime.c_str());
}

static void  PrintInstrumentInfo(const XFinApi::TradeApi::Instrument &inst)
{
	printf(" ExchangeID=%s, ProductID=%s, ID=%s, Name=%s\n",
		inst.ExchangeID.c_str(), inst.ProductID.c_str(),
		inst.InstrumentID.c_str(), inst.InstrumentName.c_str());
}

static void  PrintPositionInfo(const XFinApi::TradeApi::Position &pos)
{
	printf("  InstID=%s, Direction=%d, Price=%g, PosToday=%lld, PosYesterday=%lld, BuyPosition=%lld, SellPosition=%lld\n",
		pos.InstrumentID.c_str(), pos.Direction,
		DEFAULT_FILTER(pos.PreSettlementPrice),
		DEFAULT_FILTER(pos.PositionToday), DEFAULT_FILTER(pos.PositionYesterday),
		DEFAULT_FILTER(pos.BuyPosition), DEFAULT_FILTER(pos.SellPosition));
}

static void  PrintAccountInfo(const XFinApi::TradeApi::Account &acc)
{
	printf("  PreBalance=%g, PreCredit=%g, PreMortgage=%g, Mortgage=%g, Withdraw=%g, Deposit=%g, DeliveryMargin=%g\n",
		DEFAULT_FILTER(acc.PreBalance), DEFAULT_FILTER(acc.PreCredit), DEFAULT_FILTER(acc.PreMortgage), DEFAULT_FILTER(acc.Mortgage),
		DEFAULT_FILTER(acc.Withdraw), DEFAULT_FILTER(acc.Deposit), DEFAULT_FILTER(acc.DeliveryMargin));
}

static bool TimeIsSmaller(const std::string &lhs, const std::string &rhs)
{
	int h1, m1, s1, h2, m2, s2;
	sscanf_s(lhs.c_str(), "%d:%d:%d", &h1, &m1, &s1);
	sscanf_s(rhs.c_str(), "%d:%d:%d", &h2, &m2, &s2);

	if (h1 == h2)
	{
		if (m1 == m2)
			return s1 < s2;

		return m1 < m2;
	}
	return h1 < h2;
}

//////////////////////////////////////////////////////////////////////////////////
//API ����ʧ�ܴ�����ĺ��壬����������ĺ���μ�TradeApi_win32x86\Cpp\ApiEnum.h�ļ�
static const char *StrCreateErrors[] = {
	"�޴���",
	"ͷ�ļ���ӿڰ汾��ƥ��",
	"ͷ�ļ���ʵ�ְ汾��ƥ��",
	"ʵ�ּ���ʧ��",
	"ʵ�����δ�ҵ�",
	"����ʵ��ʧ��",
	"����Ȩ�ļ�",
	"��Ȩ�汾����",
	"���һ��ͨ�ų���",
	"���������",
	"��֤�ļ�����",
	"��֤��ʱ"
};

//////////////////////////////////////////////////////////////////////////////////
//�����¼�
class MarketEvent : public XFinApi::TradeApi::MarketListener
{
public:
	MarketEvent() {}
	~MarketEvent() {}

	void OnNotify(const XFinApi::TradeApi::NotifyParams & notifyParams) override
	{
		printf("* Market");
		PrintNotifyInfo(notifyParams);

		//���ӳɹ���ɶ��ĺ�Լ
		if ((int)XFinApi::TradeApi::Action::Open == notifyParams.Action &&
			(int)XFinApi::TradeApi::Result::Success == notifyParams.Result && market)
		{
			//����
			XFinApi::TradeApi::QueryParams param;
			param.InstrumentID = Cfg.InstrumentID;
			market->Subscribe(param);
		}

		//ToDo ...
	}

	void OnSubscribed(const XFinApi::TradeApi::QueryParams &instInfo) override
	{
		PrintSubscribedInfo(instInfo);

		//ToDo ...
	}

	void OnUnsubscribed(const XFinApi::TradeApi::QueryParams &instInfo) override
	{
		PrintUnsubscribedInfo(instInfo);

		//ToDo ...
	}

	void OnTick(const XFinApi::TradeApi::Tick &tick) override
	{
		if (Cfg.SellPrice1 <= 0 && Cfg.BuyPrice1 <= 0)
			PrintTickInfo(tick);

		Cfg.SellPrice1 = tick.AskPrice[0];
		Cfg.BuyPrice1 = tick.BidPrice[0];

		//ToDo ...
	}
};

//////////////////////////////////////////////////////////////////////////////////
//�����¼�
class TradeEvent : public XFinApi::TradeApi::TradeListener
{
public:
	TradeEvent() {}
	~TradeEvent() {}

	void OnNotify(const XFinApi::TradeApi::NotifyParams &notifyParams) override
	{
		printf("* Trade");
		PrintNotifyInfo(notifyParams);

		//ToDo ...
	}

	void OnUpdateOrder(const XFinApi::TradeApi::Order &order) override
	{
		printf("- OnUpdateOrder:\n");
		PrintOrderInfo(order);

		//ToDo ...
	}

	void OnUpdateTradeOrder(const XFinApi::TradeApi::TradeOrder &trade) override
	{
		printf("- OnUpdateTradeOrder:\n");
		PrintTradeInfo(trade);

		//ToDo ...
	}

	void OnQueryOrder(const std::vector<XFinApi::TradeApi::Order> &orders) override
	{
		printf("- OnQueryOrder:\n");

		std::vector<XFinApi::TradeApi::Order> sortedOrders = orders;
		std::sort(sortedOrders.begin(), sortedOrders.end(), [this](const XFinApi::TradeApi::Order &lhs, const XFinApi::TradeApi::Order &rhs)
		{
			return TimeIsSmaller(lhs.OrderTime, rhs.OrderTime);
		});

		for (const XFinApi::TradeApi::Order &order : sortedOrders)
		{
			PrintOrderInfo(order);

			//ToDo ...
		}
	}

	void OnQueryTradeOrder(const std::vector<XFinApi::TradeApi::TradeOrder> &trades) override
	{
		printf("- OnQueryTradeOrder:\n");

		std::vector<XFinApi::TradeApi::TradeOrder> sortedTradeOrders = trades;
		std::sort(sortedTradeOrders.begin(), sortedTradeOrders.end(), [this](const XFinApi::TradeApi::TradeOrder &lhs, const XFinApi::TradeApi::TradeOrder &rhs)
		{
			return TimeIsSmaller(lhs.TradeTime, rhs.TradeTime);
		});

		for (const XFinApi::TradeApi::TradeOrder &trade : sortedTradeOrders)
		{
			PrintTradeInfo(trade);

			//ToDo ...
		}
	}

	void OnQueryInstrument(const std::vector<XFinApi::TradeApi::Instrument> &insts) override
	{
		printf("- OnQueryInstrument:\n");

		for (const XFinApi::TradeApi::Instrument &inst : insts)
		{
			PrintInstrumentInfo(inst);

			//ToDo ...
		}
	}

	void OnQueryPosition(const std::vector<XFinApi::TradeApi::Position> &posInfos) override
	{
		printf("- OnQueryPosition\n");
		for (const XFinApi::TradeApi::Position &pos : posInfos)
			PrintPositionInfo(pos);

		//ToDo ...
	}

	void OnQueryAccount(const XFinApi::TradeApi::Account &accInfo) override
	{
		printf("- OnQueryAccount\n");
		PrintAccountInfo(accInfo);

		//ToDo ...
	}
};

//////////////////////////////////////////////////////////////////////////////////
//�������
void MarketTest()
{
	//���� IMarket
	//const char* path ָ xxx.exe ͬ����Ŀ¼�е� xxx.dll �ļ�
	int err = -1;
#ifdef _DEBUG
	market = XFinApi_CreateMarketApi("TradeApi_win32x86/Api/CTPTradeApi_v6.3.6_20160606/XFinApi.CTPTradeApid.dll", &err);
#else
	market = XFinApi_CreateMarketApi("TradeApi_win32x86/Api/CTPTradeApi_v6.3.6_20160606/XFinApi.CTPTradeApi.dll", &err);
#endif

	if (err || !market)
	{
		printf("* Market XFinApiCreateError=%s;\n", StrCreateErrors[err]);
		return;
	}

	//ע���¼�
	marketEvent = new MarketEvent();
	market->SetListener(marketEvent);

	//���ӷ�����
	//Cfg.SetNonTradeTime();//�ǽ���ʱ��
	XFinApi::TradeApi::OpenParams openParams;
	openParams.HostAddress = Cfg.MarketAddress;
	openParams.BrokerID = Cfg.BrokerID;
	openParams.UserID = Cfg.UserName;
	openParams.Password = Cfg.Password;
	market->Open(openParams);

	/*
	���ӳɹ������ִ�ж�������Ȳ�������ⷽ�������֣�
	1��IMarket::IsOpened()=true
	2��MarketListener::OnNotify��
	(int)XFinApi::TradeApi::Action::Open == notifyParams.Action &&
	(int)XFinApi::TradeApi::Result::Success == notifyParams.Result
	*/

	/* ������ط���
	while (!market->IsOpened())
		std::this_thread::sleep_for(std::chrono::seconds(1));

	//�������飬����MarketEvent::OnNotify�ж���
	XFinApi::TradeApi::QueryParams param;
	param.InstrumentID = Cfg.InstrumentID;
	market->Subscribe(param);

	//ȡ����������
	market->Unsubscribe(param);
	*/
}

//////////////////////////////////////////////////////////////////////////////////
//���ײ���
void TradeTest()
{
	//���� ITrade
	//const char* path ָ xxx.exe ͬ����Ŀ¼�е� xxx.dll �ļ�
	int err = -1;
#ifdef _DEBUG
	trade = XFinApi_CreateTradeApi("TradeApi_win32x86/Api/CTPTradeApi_v6.3.6_20160606/XFinApi.CTPTradeApid.dll", &err);
#else
	trade = XFinApi_CreateTradeApi("TradeApi_win32x86/Api/CTPTradeApi_v6.3.6_20160606/XFinApi.CTPTradeApi.dll", &err);
#endif
	if (err && !trade)
	{
		printf("* Trade XFinApiCreateError=%s;\n", StrCreateErrors[err]);
		return;
	}

	//ע���¼�
	tradeEvent = new TradeEvent;
	trade->SetListener(tradeEvent);

	//���ӷ�����
	//Cfg.SetNonTradeTime();//�ǽ���ʱ��
	XFinApi::TradeApi::OpenParams openParams;
	openParams.HostAddress = Cfg.TradeAddress;
	openParams.BrokerID = Cfg.BrokerID;
	openParams.UserID = Cfg.UserName;
	openParams.Password = Cfg.Password;
	trade->Open(openParams);

	/*
	//���ӳɹ������ִ�в�ѯ��ί�еȲ�������ⷽ�������֣�
	1��ITrade::IsOpened()=true
	2��TradeListener::OnNotify��
	(int)XFinApi::TradeApi::Action::Open == notifyParams.Action &&
	(int)XFinApi::TradeApi::Result::Success == notifyParams.Result
	 */
	while (!trade->IsOpened())
		std::this_thread::sleep_for(std::chrono::seconds(1));

	XFinApi::TradeApi::QueryParams qryParam;
	qryParam.InstrumentID = Cfg.InstrumentID;

	//��ѯί�е�
	std::this_thread::sleep_for(std::chrono::seconds(1));//��Щ�ӿڲ�ѯ�м�����ƣ��磺CTP��ѯ���Ϊ1��
	std::cout << "Press any key to QueryOrder.\n";
	getchar();
	trade->QueryOrder(qryParam);

	//��ѯ�ɽ���
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "Press any key to QueryTradeOrder.\n";
	getchar();
	trade->QueryTradeOrder(qryParam);

	//��ѯ��Լ
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "Press any key to QueryInstrument.\n";
	getchar();
	trade->QueryInstrument(qryParam);

	//��ѯ�ֲ�
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "Press any key to QueryPosition.\n";
	getchar();
	trade->QueryPosition(qryParam);

	//��ѯ�˻�
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "Press any key to QueryAccount.\n";
	getchar();
	trade->QueryAccount(qryParam);

	//ί���µ�
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "Press any key to OrderAction.\n";
	getchar();
	XFinApi::TradeApi::Order order;
	order.InstrumentID = Cfg.InstrumentID;
	order.Price = Cfg.SellPrice1;
	order.Volume = 1;
	order.Direction = XFinApi::TradeApi::TradeDirection::Buy;
	order.OpenCloseType = XFinApi::TradeApi::OpenCloseKind::Open;

	//�µ��߼�ѡ���ѡ��������
	order.ActionType = XFinApi::TradeApi::ActionKind::Insert;//�µ�
	order.OrderType = XFinApi::TradeApi::OrderKind::Order;//��׼��
	order.PriceCond = XFinApi::TradeApi::PriceCondition::LimitPrice;//�޼�
	order.VolumeCond = XFinApi::TradeApi::VolumeCondition::AnyVolume;//��������
	order.TimeCond = XFinApi::TradeApi::TimeCondition::GFD;//������Ч
	order.ContingentCond = XFinApi::TradeApi::ContingentCondKind::Immediately;//����
	order.HedgeType = XFinApi::TradeApi::HedgeKind::Speculation;//Ͷ��
	order.ExecResult = XFinApi::TradeApi::ExecResultKind::NoExec;//û��ִ��

	trade->OrderAction(order);
}

int main()
{
	//����Config�����޸��û��������롢��Լ����Ϣ

	MarketTest();
	TradeTest();

	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "Press any key to close.\n";
	getchar();

	//�ر�����
	if (market)
	{
		market->Close();
		XFinApi_ReleaseMarketApi(market);//�����ͷ���Դ
	}
	if (trade)
	{
		trade->Close();
		XFinApi_ReleaseTradeApi(trade);//�����ͷ���Դ
	}
	//�����¼�
	if (marketEvent)
	{
		delete marketEvent;
		marketEvent = nullptr;
	}
	if (tradeEvent)
	{
		delete tradeEvent;
		tradeEvent = nullptr;
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "Closed.\n";
	getchar();

	return 0;
}